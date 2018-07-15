#include <base/source/fstreamer.h>
#include <public.sdk/source/vst/vstaudioprocessoralgo.h>

#include "Messaging.h"

#include "VAC6Processor.h"
#include "VAC6CIDs.h"

namespace pongasoft {
namespace VST {

using namespace Common;
using namespace VAC6;

/////////////////////////////////////////
// VAC6AudioChannelProcessor::genericProcessChannel
/////////////////////////////////////////
template<typename SampleType>
bool VAC6AudioChannelProcessor::genericProcessChannel(ZoomWindow const *iZoomWindow,
                                                      typename AudioBuffers<SampleType>::Channel const &iIn,
                                                      typename AudioBuffers<SampleType>::Channel &iOut,
                                                      double const &iGain)
{
  DCHECK_EQ_F(iIn.getNumSamples(), iOut.getNumSamples());

  if(fNeedToRecomputeZoomMaxBuffer)
  {
    fZoomMaxAccumulator = iZoomWindow->computeZoomWindow(*fMaxBuffer, *fZoomMaxBuffer);
    fNeedToRecomputeZoomMaxBuffer = false;
  }

  bool silent = true;

  auto numSamples = iIn.getNumSamples();
  auto inPtr = iIn.getBuffer();
  auto outPtr = iOut.getBuffer();

  for(int i = 0; i < numSamples; ++i, inPtr++, outPtr++)
  {
    TSample sample = *inPtr;

    if(iGain != Gain::Unity)
      sample *= iGain;

    if(fIsLiveView)
    {
      TSample max;
      if(fMaxAccumulatorForBuffer.accumulate(sample, max))
      {
        fMaxBuffer->push(max);

        // only when we get a sample in the max buffer do we accumulate in the zoomed one
        TSample zoomedMax;
        if(fZoomMaxAccumulator.accumulate(max, zoomedMax))
        {
          fZoomMaxBuffer->push(zoomedMax);
          if(zoomedMax > fMaxLevelSinceReset)
          {
            fMaxLevelSinceReset = zoomedMax;
          }
        }
      }
    }

    if(silent && !pongasoft::VST::Common::isSilent(sample))
      silent = false;

    *outPtr = sample;
  }

  iOut.setSilenceFlag(silent);

  return silent;
}

///////////////////////////////////////////
// VAC6Processor::VAC6Processor
///////////////////////////////////////////
VAC6Processor::VAC6Processor() :
  AudioEffect(),
  fMaxLevelResetRequested{false},
  fState{},
  fPreviousState{fState},
  fGain{fState.fGain1.getValue() * fState.fGain2.getValue(), DEFAULT_GAIN_FILTER},
  fStateUpdate{},
  fLatestState{fState},
  fClock{44100},
  fMaxAccumulatorBatchSize{fClock.getSampleCountFor(ACCUMULATOR_BATCH_SIZE_IN_MS)},
  fZoomWindow{nullptr},
  fLeftChannelProcessor{nullptr},
  fRightChannelProcessor{nullptr},
  fTimer{nullptr},
  fRateLimiter{},
  fLCDDataUpdate{}
{
  setControllerClass(VAC6ControllerUID);
  DLOG_F(INFO, "VAC6Processor::VAC6Processor()");
}

///////////////////////////////////////////
// VAC6Processor::~VAC6Processor
///////////////////////////////////////////
VAC6Processor::~VAC6Processor()
{
  DLOG_F(INFO, "VAC6Processor::~VAC6Processor()");

  delete fRightChannelProcessor;
  delete fLeftChannelProcessor;
  delete fZoomWindow;
}


///////////////////////////////////////////
// VAC6Processor::initialize
///////////////////////////////////////////
tresult PLUGIN_API VAC6Processor::initialize(FUnknown *context)
{
  DLOG_F(INFO, "VAC6Processor::initialize()");

  tresult result = AudioEffect::initialize(context);
  if(result != kResultOk)
  {
    return result;
  }

  addAudioInput(STR16 ("Stereo In"), SpeakerArr::kStereo);
  addAudioOutput(STR16 ("Stereo Out"), SpeakerArr::kStereo);

  return result;
}

///////////////////////////////////////////
// VAC6Processor::terminate
///////////////////////////////////////////
tresult PLUGIN_API VAC6Processor::terminate()
{
  DLOG_F(INFO, "VAC6Processor::terminate()");

  return AudioEffect::terminate();
}

///////////////////////////////////////////
// VAC6Processor::setupProcessing
///////////////////////////////////////////
tresult VAC6Processor::setupProcessing(ProcessSetup &setup)
{
  tresult result = AudioEffect::setupProcessing(setup);

  if(result != kResultOk)
    return result;

  fClock.setSampleRate(setup.sampleRate);

  fRateLimiter = fClock.getRateLimiter(UI_FRAME_RATE_MS);

  // since this method is called multiple times, we make sure that there is no leak...
  delete fRightChannelProcessor;
  delete fLeftChannelProcessor;
  delete fZoomWindow;

  fMaxAccumulatorBatchSize = fClock.getSampleCountFor(ACCUMULATOR_BATCH_SIZE_IN_MS);

  fZoomWindow = new ZoomWindow(MAX_ARRAY_SIZE, SAMPLE_BUFFER_SIZE);
  fLeftChannelProcessor = new VAC6AudioChannelProcessor(fClock, fZoomWindow, SAMPLE_BUFFER_SIZE);
  fRightChannelProcessor = new VAC6AudioChannelProcessor(fClock, fZoomWindow, SAMPLE_BUFFER_SIZE);

  DLOG_F(INFO,
         "VAC6Processor::setupProcessing(processMode=%d, symbolicSampleSize=%d, maxSamplesPerBlock=%d, sampleRate=%f, accumulatorBatchSize=%d)",
         setup.processMode,
         setup.symbolicSampleSize,
         setup.maxSamplesPerBlock,
         setup.sampleRate,
         fMaxAccumulatorBatchSize);

  return result;
}

///////////////////////////////////////////
// VAC6Processor::setActive
///////////////////////////////////////////
tresult PLUGIN_API VAC6Processor::setActive(TBool state)
{
  DLOG_F(INFO, "VAC6Processor::setActive(%s)", state ? "true" : "false");

  if(fTimer != nullptr)
  {
    fTimer->release();
    fTimer = nullptr;
  }

  if(state != 0)
  {
    fTimer = Timer::create(this, UI_FRAME_RATE_MS);
  }

  return AudioEffect::setActive(state);
}

///////////////////////////////////////////
// VAC6Processor::process
///////////////////////////////////////////
tresult PLUGIN_API VAC6Processor::process(ProcessData &data)
{
  // 1. we check if there was any state update (UI calls setState)
  fStateUpdate.pop(fState);

  // 2. process parameter changes (this will override any update in step 1.)
  if(data.inputParameterChanges != nullptr)
    processParameters(*data.inputParameterChanges);

  // 3. process inputs
  tresult res = processInputs(data);

  // 4. update the previous state
  fPreviousState = fState;

  return res;
}

/////////////////////////////////////////
// VAC6Processor::genericProcessInputs
/////////////////////////////////////////
template<typename SampleType>
tresult VAC6Processor::genericProcessInputs(ProcessData &data)
{
  AudioBuffers<SampleType> in(data.inputs[0], data.numSamples);
  AudioBuffers<SampleType> out(data.outputs[0], data.numSamples);

  // Handling mono or stereo only
  if(in.getNumChannels() < 1 || in.getNumChannels() > 2 || out.getNumChannels() < 1 || out.getNumChannels() > 2)
    return kResultFalse;

  bool isNewLiveView = false;
  bool isNewPause = false;

  // live view/pause has changed
  if(fPreviousState.fLCDLiveView != fState.fLCDLiveView)
  {
    fLeftChannelProcessor->setIsLiveView(fState.fLCDLiveView);
    fRightChannelProcessor->setIsLiveView(fState.fLCDLiveView);

    isNewLiveView = fState.fLCDLiveView;
    isNewPause =!isNewLiveView;
  }

  // Gain filter has changed
  if(fPreviousState.fGainFilter != fState.fGainFilter)
  {
    fGain.setFilterOn(fState.fGainFilter);
  }

    // gain has changed
  if(fPreviousState.fGain1.getValue() != fState.fGain1.getValue() ||
     fPreviousState.fGain2.getValue() != fState.fGain2.getValue())
  {
    // simply combine the 2 gains
    fGain.setTargetValue(fState.fGain1.getValue() * fState.fGain2.getValue());
  }

  // Zoom has changed
  if(fPreviousState.fZoomFactorX != fState.fZoomFactorX)
  {
    if(fState.fLCDLiveView)
    {
      fZoomWindow->setZoomFactor(fState.fZoomFactorX);
    }
    else
    {
      int newLCDInputX =
        fZoomWindow->setZoomFactor(fState.fZoomFactorX,
                                   fState.fLCDInputX != LCD_INPUT_X_NOTHING_SELECTED ? fState.fLCDInputX : MAX_ARRAY_SIZE / 2,
                                   { fState.fLeftChannelOn ? &fLeftChannelProcessor->getMaxBuffer() : nullptr,
                                     fState.fRightChannelOn ? &fRightChannelProcessor->getMaxBuffer() : nullptr });

      if(fState.fLCDInputX != LCD_INPUT_X_NOTHING_SELECTED && fState.fLCDInputX != newLCDInputX)
      {
        fState.updateLCDInputX(data, newLCDInputX);
      }

      double newLCDHistoryOffset = fZoomWindow->getWindowOffset();

      if(newLCDHistoryOffset != fState.fLCDHistoryOffset)
      {
        fState.updateLCDHistoryOffset(data, newLCDHistoryOffset);
      }
    }

    fLeftChannelProcessor->setDirty();
    fRightChannelProcessor->setDirty();
  }

  // Scrollbar has been moved (should happen only in pause mode)
  if(fPreviousState.fLCDHistoryOffset != fState.fLCDHistoryOffset)
  {
    fZoomWindow->setWindowOffset(fState.fLCDHistoryOffset);
    fLeftChannelProcessor->setDirty();
    fRightChannelProcessor->setDirty();
  }

  // after we cancel pause we need to reset LCDInputX and LCDHistoryOffset
  if(isNewLiveView)
  {
    if(fState.fLCDInputX != LCD_INPUT_X_NOTHING_SELECTED)
    {
      fState.updateLCDInputX(data, LCD_INPUT_X_NOTHING_SELECTED);
    }

    if(fState.fLCDHistoryOffset != MAX_HISTORY_OFFSET)
    {
      fState.updateLCDHistoryOffset(data, MAX_HISTORY_OFFSET);
      fZoomWindow->setWindowOffset(fState.fLCDHistoryOffset);
      fLeftChannelProcessor->setDirty();
      fRightChannelProcessor->setDirty();
    }
  }

  // we need to adjust the filtered gain
  fGain.adjust();
  auto gain = fState.fBypass ? Gain::Unity : fGain.getValue();

  // in mono case there could be only one channel
  auto leftChannel = out.getLeftChannel();
  fLeftChannelProcessor->genericProcessChannel<SampleType>(fZoomWindow, in.getLeftChannel(), leftChannel, gain);

  if(in.getNumChannels() == 2)
  {
    auto rightChannel = out.getNumChannels() == 2 ? out.getRightChannel() : leftChannel;
    fRightChannelProcessor->genericProcessChannel<SampleType>(fZoomWindow, in.getRightChannel(), rightChannel, gain);
  }

  // if reset of max level is requested (pressing momentary button) then we need to reset the accumulator
  if(fMaxLevelResetRequested)
  {
    fLeftChannelProcessor->resetMaxLevelSinceReset();
    fRightChannelProcessor->resetMaxLevelSinceReset();
  }

  // is it time to update the UI?
  if(isNewPause || fRateLimiter.shouldUpdate(static_cast<uint32>(data.numSamples)))
  {
    LCDData lcdData{};

    // left
    if(fState.fLeftChannelOn)
    {
      fLeftChannelProcessor->computeZoomSamples(MAX_ARRAY_SIZE, lcdData.fLeftChannel.fSamples);
      lcdData.fLeftChannel.fMaxLevelSinceReset = fLeftChannelProcessor->getMaxLevelSinceReset();
    }
    lcdData.fLeftChannel.fOn = fState.fLeftChannelOn;


    // right
    if(fState.fRightChannelOn)
    {
      fRightChannelProcessor->computeZoomSamples(MAX_ARRAY_SIZE, lcdData.fRightChannel.fSamples);
      lcdData.fRightChannel.fMaxLevelSinceReset = fRightChannelProcessor->getMaxLevelSinceReset();
    }
    lcdData.fRightChannel.fOn = fState.fRightChannelOn;

    lcdData.fWindowSizeInMillis = getWindowSizeInMillis();

    fLCDDataUpdate.push(lcdData);
  }

  return kResultOk;
}

///////////////////////////////////////////
// VAC6Processor::processInputs
///////////////////////////////////////////
tresult VAC6Processor::processInputs(ProcessData &data)
{
  // 2. process inputs
  if(data.numInputs == 0 || data.numOutputs == 0)
  {
    // nothing to do
    return kResultOk;
  }

  if(data.symbolicSampleSize == kSample32)
    return genericProcessInputs<Sample32>(data);
  else
    return genericProcessInputs<Sample64>(data);
}

///////////////////////////////////////////
// VAC6Processor::canProcessSampleSize
//
// * Overridden so that we can declare we support 64bits
///////////////////////////////////////////
tresult VAC6Processor::canProcessSampleSize(int32 symbolicSampleSize)
{
  if(symbolicSampleSize == kSample32)
    return kResultTrue;

  // we support double processing
  if(symbolicSampleSize == kSample64)
    return kResultTrue;

  return kResultFalse;
}

///////////////////////////////////////////
// VAC6Processor::processParameters
///////////////////////////////////////////
bool VAC6Processor::processParameters(IParameterChanges &inputParameterChanges)
{
  int32 numParamsChanged = inputParameterChanges.getParameterCount();
  if(numParamsChanged <= 0)
    return false;

  bool stateChanged = false;
  State newState{fState};

  for(int i = 0; i < numParamsChanged; ++i)
  {
    IParamValueQueue *paramQueue = inputParameterChanges.getParameterData(i);
    if(paramQueue != nullptr)
    {
      ParamValue value;
      int32 sampleOffset;
      int32 numPoints = paramQueue->getPointCount();

      // we read the "last" point (ignoring multiple changes for now)
      if(paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultOk)
      {
        switch(paramQueue->getParameterId())
        {
          case kBypass:
            newState.fBypass = BooleanParamConverter::denormalize(value);
            stateChanged |= newState.fBypass != fState.fBypass;
            break;

          case kMaxLevelReset:
            fMaxLevelResetRequested = BooleanParamConverter::denormalize(value);
            break;

          case kLCDZoomFactorX:
            newState.fZoomFactorX = LCDZoomFactorXParamConverter::denormalize(value);
            stateChanged |= newState.fZoomFactorX != fState.fZoomFactorX;
            break;

          case kLCDLeftChannel:
            newState.fLeftChannelOn = BooleanParamConverter::denormalize(value);
            stateChanged |= newState.fLeftChannelOn != fState.fLeftChannelOn;
            break;

          case kLCDRightChannel:
            newState.fRightChannelOn = BooleanParamConverter::denormalize(value);
            stateChanged |= newState.fRightChannelOn != fState.fRightChannelOn;
            break;

          case kLCDLiveView:
            newState.fLCDLiveView = BooleanParamConverter::denormalize(value);
            stateChanged |= newState.fLCDLiveView != fState.fLCDLiveView;
            break;

          case kLCDInputX:
            newState.fLCDInputX = LCDInputXParamConverter::denormalize(value);
            stateChanged |= newState.fLCDInputX != fState.fLCDInputX;
            break;

          case kLCDHistoryOffset:
            newState.fLCDHistoryOffset = LCDHistoryOffsetParamConverter::denormalize(value);
            stateChanged |= newState.fLCDHistoryOffset != fState.fLCDHistoryOffset;
            break;

          case kGain1:
            newState.fGain1 = GainParamConverter::denormalize(value);
            stateChanged |= newState.fGain1.getValue() != fState.fGain1.getValue();
            break;

          case kGain2:
            newState.fGain2 = GainParamConverter::denormalize(value);
            stateChanged |= newState.fGain2.getValue() != fState.fGain2.getValue();
            break;

          case kGainFilter:
            newState.fGainFilter = BooleanParamConverter::denormalize(value);
            stateChanged |= newState.fGainFilter != fState.fGainFilter;
            break;

          default:
            // shouldn't happen?
            break;
        }
      }
    }
  }

  if(stateChanged)
  {
    fState = newState;
    fLatestState.set(newState);
  }

  return stateChanged;
}

///////////////////////////////////
// ::readParam
///////////////////////////////////
template<typename ParamConverter>
void readParam(IBStreamer &iStreamer, const typename ParamConverter::ParamType &iDefaultValue, typename ParamConverter::ParamType &oValue)
{
  double value;
  if(iStreamer.readDouble(value))
    oValue = ParamConverter::denormalize(value);
  else
    oValue = iDefaultValue;
}

///////////////////////////////////
// VAC6Processor::setState
///////////////////////////////////
tresult VAC6Processor::setState(IBStream *state)
{
  if(state == nullptr)
    return kResultFalse;

  State newState{};

  IBStreamer streamer(state, kLittleEndian);

  uint16 stateVersion;
  if(!streamer.readInt16u(stateVersion))
    stateVersion = PROCESSOR_STATE_VERSION;

  if(stateVersion != PROCESSOR_STATE_VERSION)
  {
    DLOG_F(WARNING, "unexpected processor state version %d", stateVersion);
  }

  readParam<LCDZoomFactorXParamConverter>(streamer, DEFAULT_ZOOM_FACTOR_X, newState.fZoomFactorX);
  readParam<BooleanParamConverter>(streamer, true, newState.fLeftChannelOn);
  readParam<BooleanParamConverter>(streamer, true, newState.fRightChannelOn);
  readParam<GainParamConverter>(streamer, DEFAULT_GAIN, newState.fGain1);
  readParam<GainParamConverter>(streamer, DEFAULT_GAIN, newState.fGain2);
  readParam<BooleanParamConverter>(streamer, true, newState.fGainFilter);
  readParam<BooleanParamConverter>(streamer, false, newState.fBypass);

  // lcd live view IGNORED! (does not make sense to not be in live view when loading)

  fStateUpdate.push(newState);

  return kResultOk;
}

///////////////////////////////////
// ::readParam
///////////////////////////////////
template<typename ParamConverter>
void writeParam(IBStreamer &iStreamer, typename ParamConverter::ParamType const &iValue)
{
  iStreamer.writeDouble(ParamConverter::normalize(iValue));
}

///////////////////////////////////
// VAC6Processor::getState
///////////////////////////////////
tresult VAC6Processor::getState(IBStream *state)
{
  if(state == nullptr)
    return kResultFalse;

  auto latestState = fLatestState.get();

  IBStreamer streamer(state, kLittleEndian);

  // write version for later upgrade
  streamer.writeInt16u(PROCESSOR_STATE_VERSION);

  writeParam<LCDZoomFactorXParamConverter>(streamer, latestState.fZoomFactorX);
  writeParam<BooleanParamConverter>(streamer, latestState.fLeftChannelOn);
  writeParam<BooleanParamConverter>(streamer, latestState.fRightChannelOn);
  writeParam<GainParamConverter>(streamer, latestState.fGain1);
  writeParam<GainParamConverter>(streamer, latestState.fGain2);
  writeParam<BooleanParamConverter>(streamer, latestState.fGainFilter);
  writeParam<BooleanParamConverter>(streamer, latestState.fBypass);

  return kResultOk;
}

///////////////////////////////////////////
// VAC6Processor::onTimer
///////////////////////////////////////////
void VAC6Processor::onTimer(Timer * /* timer */)
{
  LCDData lcdData{};
  if(fLCDDataUpdate.pop(lcdData))
  {
    if(auto message = owned(allocateMessage()))
    {
      Message m{message};

      m.setMessageID(kLCDData_MID);

      if(lcdData.fLeftChannel.fOn)
        m.setBinary(LCDDATA_LEFT_SAMPLES_ATTR, lcdData.fLeftChannel.fSamples, MAX_ARRAY_SIZE);
      m.setFloat(LCDDATA_LEFT_MAX_LEVEL_SINCE_RESET_ATTR, lcdData.fLeftChannel.fMaxLevelSinceReset);

      if(lcdData.fRightChannel.fOn)
        m.setBinary(LCDDATA_RIGHT_SAMPLES_ATTR, lcdData.fRightChannel.fSamples, MAX_ARRAY_SIZE);
      m.setFloat(LCDDATA_RIGHT_MAX_LEVEL_SINCE_RESET_ATTR, lcdData.fRightChannel.fMaxLevelSinceReset);

      m.setInt(LCDDATA_WINDOW_SIZE_MS_ATTR, lcdData.fWindowSizeInMillis);

      sendMessage(message);
    }
  }
}

///////////////////////////////////////////
// VAC6Processor::State::updateLCDInputX
///////////////////////////////////////////
void VAC6Processor::State::updateLCDInputX(ProcessData &iData, int iLCDInputX)
{
  fLCDInputX = iLCDInputX;
  addOutputParameterChange(iData, EVAC6ParamID::kLCDInputX, LCDInputXParamConverter::normalize(fLCDInputX));
}

///////////////////////////////////////////
// VAC6Processor::State::updateLCDHistoryOffset
///////////////////////////////////////////
void VAC6Processor::State::updateLCDHistoryOffset(ProcessData &iData, double iLCDHistoryOffset)
{
  fLCDHistoryOffset = iLCDHistoryOffset;
  addOutputParameterChange(iData, EVAC6ParamID::kLCDHistoryOffset, LCDHistoryOffsetParamConverter::normalize(fLCDHistoryOffset));

}
}
}