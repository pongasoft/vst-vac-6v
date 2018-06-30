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
bool VAC6AudioChannelProcessor::genericProcessChannel(typename AudioBuffers<SampleType>::Channel const &iIn,
                                                      typename AudioBuffers<SampleType>::Channel &iOut)
{
  DCHECK_EQ_F(iIn.getNumSamples(), iOut.getNumSamples());

  if(fNeedToRecomputeZoomMaxBuffer)
  {
    fZoomMaxAccumulator = fZoomWindow.computeZoomWindow(*fMaxBuffer, *fZoomMaxBuffer);
    if(!fIsLiveView)
    {
      fMaxLevel = fZoomMaxBuffer->getAt(fPausedZoomMaxBufferOffset);
    }
    fNeedToRecomputeZoomMaxBuffer = false;
  }

  bool silent = true;

  auto numSamples = iIn.getNumSamples();
  auto inPtr = iIn.getBuffer();
  auto outPtr = iOut.getBuffer();

  for(int i = 0; i < numSamples; ++i, inPtr++, outPtr++)
  {
    const TSample sample = *inPtr;

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
        }
      }

      if(fMaxLevelAccumulator.accumulate(sample, max))
      {
        fMaxLevel = max;
      }
      else
      {
        fMaxLevel = fMaxLevelAccumulator.getAccumulatedMax();
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
  fStateUpdate{},
  fLatestState{fState},
  fClock{44100},
  fLeftChannelProcessor{nullptr},
  fRightChannelProcessor{nullptr},
  fTimer{nullptr},
  fRateLimiter{},
  fMaxLevelUpdate{},
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

  fLeftChannelProcessor = new VAC6AudioChannelProcessor(fClock);
  fRightChannelProcessor = new VAC6AudioChannelProcessor(fClock);

  DLOG_F(INFO,
         "VAC6Processor::setupProcessing(processMode=%d, symbolicSampleSize=%d, maxSamplesPerBlock=%d, sampleRate=%f, fMaxBufferSize=%d, accumulatorBatchSize=%ld)",
         setup.processMode,
         setup.symbolicSampleSize,
         setup.maxSamplesPerBlock,
         setup.sampleRate,
         fLeftChannelProcessor->getMaxBuffer().getSize(),
         fLeftChannelProcessor->getBufferAccumulatorBatchSize());

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

  // making sure we are being called properly...
  if(in.getNumChannels() != 2 || out.getNumChannels() != 2)
    return kResultFalse;

  bool isNewLiveView = false;

  // live view/pause has changed
  if(fPreviousState.fLCDLiveView != fState.fLCDLiveView)
  {
    fLeftChannelProcessor->setIsLiveView(fState.fLCDLiveView);
    fRightChannelProcessor->setIsLiveView(fState.fLCDLiveView);

    isNewLiveView = fState.fLCDLiveView;
  }

  // LCDInputX (lcd selection) has changed
  if(fPreviousState.fLCDInputX != fState.fLCDInputX)
  {
    fLeftChannelProcessor->setLCDInputX(fState.fLCDInputX);
    fRightChannelProcessor->setLCDInputX(fState.fLCDInputX);
  }

  // Max Level Auto reset selection has changed
  if(fPreviousState.fMaxLevelAutoResetInSeconds != fState.fMaxLevelAutoResetInSeconds)
  {
    fLeftChannelProcessor->resetMaxLevelAccumulator(fState.fMaxLevelAutoResetInSeconds);
    fRightChannelProcessor->resetMaxLevelAccumulator(fState.fMaxLevelAutoResetInSeconds);
  }

  // Zoom has changed
  if(fPreviousState.fZoomFactorX != fState.fZoomFactorX)
  {
    if(fState.fLCDLiveView)
    {
      fLeftChannelProcessor->setZoomFactor(fState.fZoomFactorX);
      fRightChannelProcessor->setZoomFactor(fState.fZoomFactorX);
    }
    else
    {
      fLeftChannelProcessor->setZoomFactor(fState.fZoomFactorX);
      fRightChannelProcessor->setZoomFactor(fState.fZoomFactorX);
//      fLeftChannelProcessor->setZoomFactor(fState.fZoomFactorX, fState.fLCDInputX);
//      fRightChannelProcessor->setZoomFactor(fState.fZoomFactorX, fState.fLCDInputX);
    }
  }

  // Scrollbar has been moved (should happen only in pause mode)
  if(fPreviousState.fLCDHistoryOffset != fState.fLCDHistoryOffset)
  {
    fLeftChannelProcessor->setHistoryOffset(fState.fLCDHistoryOffset);
    fRightChannelProcessor->setHistoryOffset(fState.fLCDHistoryOffset);
  }

  // after we cancel pause we need to reset LCDInputX and LCDHistoryOffset
  if(isNewLiveView)
  {
    if(fState.fLCDInputX != MAX_LCD_INPUT_X)
    {
      fState.fLCDInputX = MAX_LCD_INPUT_X;
      fLeftChannelProcessor->setLCDInputX(fState.fLCDInputX);
      fRightChannelProcessor->setLCDInputX(fState.fLCDInputX);
      addOutputParameterChange(data, EVAC6ParamID::kLCDInputX, LCDInputXParamConverter::normalize(fState.fLCDInputX));
    }

    if(fState.fLCDHistoryOffset != MAX_HISTORY_OFFSET)
    {
      fState.fLCDHistoryOffset = MAX_HISTORY_OFFSET;
      fLeftChannelProcessor->setHistoryOffset(fState.fLCDHistoryOffset);
      fRightChannelProcessor->setHistoryOffset(fState.fLCDHistoryOffset);
      addOutputParameterChange(data, EVAC6ParamID::kLCDHistoryOffset, LCDHistoryOffsetParamConverter::normalize(fState.fLCDHistoryOffset));
    }
  }

  auto leftChannel = out.getLeftChannel();
  auto rightChannel = out.getRightChannel();

  fLeftChannelProcessor->genericProcessChannel<SampleType>(in.getLeftChannel(), leftChannel);
  fRightChannelProcessor->genericProcessChannel<SampleType>(in.getRightChannel(), rightChannel);

  // if reset of max level is requested (pressing momentary button) then we need to reset the accumulator
  if(fState.fLCDLiveView && fMaxLevelResetRequested)
  {
    fLeftChannelProcessor->resetMaxLevelAccumulator();
    fRightChannelProcessor->resetMaxLevelAccumulator();
  }

  // is it time to update the UI?
  if(fRateLimiter.shouldUpdate(static_cast<uint32>(data.numSamples)))
  {
    LCDData lcdData{};
    if(fState.fLeftChannelOn)
      fLeftChannelProcessor->computeZoomSamples(MAX_ARRAY_SIZE, lcdData.fLeftSamples);
    lcdData.fLeftChannelOn = fState.fLeftChannelOn;
    if(fState.fRightChannelOn)
      fRightChannelProcessor->computeZoomSamples(MAX_ARRAY_SIZE, lcdData.fRightSamples);
    lcdData.fRightChannelOn = fState.fRightChannelOn;
    lcdData.fWindowSizeInMillis = fLeftChannelProcessor->getWindowSizeInMillis();

    fMaxLevelUpdate.push(MaxLevel{fLeftChannelProcessor->getMaxLevel(),
                                  fRightChannelProcessor->getMaxLevel()});
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
          case kSoftClippingLevel:
            newState.fSoftClippingLevel = SoftClippingLevel::denormalize(value);
            stateChanged |= newState.fSoftClippingLevel.getValueInSample() != fState.fSoftClippingLevel.getValueInSample();
            break;

          case kMaxLevelReset:
            fMaxLevelResetRequested = BooleanParamConverter::denormalize(value);
            break;

          case kMaxLevelAutoReset:
            newState.fMaxLevelAutoResetInSeconds = static_cast<uint32>(MaxLevelAutoResetParamConverter::denormalize(value));
            stateChanged |= newState.fMaxLevelAutoResetInSeconds != fState.fMaxLevelAutoResetInSeconds;
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
// VAC6Processor::setState
///////////////////////////////////
tresult VAC6Processor::setState(IBStream *state)
{
  if(state == nullptr)
    return kResultFalse;

  State newState{};

  IBStreamer streamer(state, kLittleEndian);

  // soft clipping level
  {
    double savedParam = 0;
    if(!streamer.readDouble(savedParam))
      savedParam = DEFAULT_SOFT_CLIPPING_LEVEL;
    newState.fSoftClippingLevel = SoftClippingLevel{savedParam};
  }

  // zoom factor X
  {
    double savedParam = 0;
    if(!streamer.readDouble(savedParam))
      savedParam = DEFAULT_ZOOM_FACTOR_X;
    newState.fZoomFactorX = savedParam;
  }

  // max level auto reset
  {
    int32 savedParam = 0;
    if(!streamer.readInt32(savedParam))
      savedParam = DEFAULT_MAX_LEVEL_RESET_IN_SECONDS;
    newState.fMaxLevelAutoResetInSeconds = static_cast<uint32>(savedParam);
  }

  // left channel on
  {
    bool savedParam;
    if(!streamer.readBool(savedParam))
      savedParam = true;
    newState.fLeftChannelOn = savedParam;
  }

  // right channel on
  {
    bool savedParam;
    if(!streamer.readBool(savedParam))
      savedParam = true;
    newState.fRightChannelOn = savedParam;
  }

  // lcd live view IGNORED! (does not make sense to not be in live view when loading)

  fStateUpdate.push(newState);

  DLOG_F(INFO, "VAC6Processor::setState => fSoftClippingLevel=%f, fZoomFactorX=%f, fMaxLevelAutoResetInSeconds=%d, fLeftChannelOn=%d, fRightChannelOn=%d",
         newState.fSoftClippingLevel.getValueInSample(),
         newState.fZoomFactorX,
         newState.fMaxLevelAutoResetInSeconds,
         newState.fLeftChannelOn,
         newState.fRightChannelOn);

  return kResultOk;
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

  streamer.writeDouble(latestState.fSoftClippingLevel.getValueInSample());
  streamer.writeDouble(latestState.fZoomFactorX);
  streamer.writeInt32(latestState.fMaxLevelAutoResetInSeconds);
  streamer.writeBool(latestState.fLeftChannelOn);
  streamer.writeBool(latestState.fRightChannelOn);

  DLOG_F(INFO, "VAC6Processor::getState => fSoftClippingLevel=%f, fZoomFactorX=%f, fMaxLevelAutoResetInSeconds=%d, fLeftChannelOn=%d, fRightChannelOn=%d",
         latestState.fSoftClippingLevel.getValueInSample(),
         latestState.fZoomFactorX,
         latestState.fMaxLevelAutoResetInSeconds,
         latestState.fLeftChannelOn,
         latestState.fRightChannelOn);

  return kResultOk;
}

///////////////////////////////////////////
// VAC6Processor::onTimer
///////////////////////////////////////////
void VAC6Processor::onTimer(Timer * /* timer */)
{
  MaxLevel maxLevel{};
  if(fMaxLevelUpdate.pop(maxLevel))
  {
    if(auto message = owned(allocateMessage()))
    {
      Message m{message};

      m.setMessageID(kMaxLevel_MID);
      m.setFloat(MAX_LEVEL_LEFT_VALUE_ATTR, maxLevel.fLeftValue);
      m.setFloat(MAX_LEVEL_RIGHT_VALUE_ATTR, maxLevel.fRightValue);

      sendMessage(message);
    }
  }

  LCDData lcdData{};
  if(fLCDDataUpdate.pop(lcdData))
  {
    if(auto message = owned(allocateMessage()))
    {
      Message m{message};

      m.setMessageID(kLCDData_MID);
      if(lcdData.fLeftChannelOn)
        m.setBinary(LCDDATA_LEFT_SAMPLES_ATTR, lcdData.fLeftSamples, MAX_ARRAY_SIZE);
      if(lcdData.fRightChannelOn)
        m.setBinary(LCDDATA_RIGHT_SAMPLES_ATTR, lcdData.fRightSamples, MAX_ARRAY_SIZE);
      m.setInt(LCDDATA_WINDOW_SIZE_MS_ATTR, lcdData.fWindowSizeInMillis);

      sendMessage(message);
    }
  }
}


}
}