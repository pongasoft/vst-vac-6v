#include <base/source/fstreamer.h>
#include <public.sdk/source/vst/vstaudioprocessoralgo.h>

#include "Messaging.h"

#include "VAC6Processor.h"
#include "VAC6CIDs.h"

namespace pongasoft {
namespace VST {

using namespace Common;
using namespace VAC6;

///////////////////////////////////////////
// VAC6Processor::VAC6Processor
///////////////////////////////////////////
VAC6Processor::VAC6Processor() :
  AudioEffect(),
  fMaxLevel{0, kStateOk},
  fMaxLevelResetRequested{false},
  fState{},
  fPreviousState{fState},
  fStateUpdate{},
  fLatestState{fState},
  fLeftMaxBuffer{nullptr},
  fLeftMaxAccumulator{nullptr},
  fRightMaxBuffer{nullptr},
  fRightMaxAccumulator{nullptr},
  fZoomWindow{nullptr},
  fClock{44100},
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

  delete fZoomWindow;
  delete fLeftMaxBuffer;
  delete fLeftMaxAccumulator;
  delete fRightMaxBuffer;
  delete fRightMaxAccumulator;
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

  DLOG_F(INFO,
         "VAC6Processor::setupProcessing(processMode=%d, symbolicSampleSize=%d, maxSamplesPerBlock=%d, sampleRate=%f)",
         setup.processMode,
         setup.symbolicSampleSize,
         setup.maxSamplesPerBlock,
         setup.sampleRate);

  fClock.setSampleRate(setup.sampleRate);

  fRateLimiter = fClock.getRateLimiter(UI_FRAME_RATE_MS);

  // since this method is called multiple times, we make sure that there is no leak...
  delete fZoomWindow;
  delete fLeftMaxBuffer;
  delete fLeftMaxAccumulator;
  delete fRightMaxBuffer;
  delete fRightMaxAccumulator;

  auto accumulatorBatchSize = static_cast<int32>(fClock.getSampleCountFor(ACCUMULATOR_BATCH_SIZE_IN_MS));

  fLeftMaxBuffer = new CircularBuffer<TSample>(static_cast<int>(ceil(fClock.getSampleCountFor(HISTORY_SIZE_IN_SECONDS * 1000) / accumulatorBatchSize)));
  fLeftMaxBuffer->init(0);
  fLeftMaxAccumulator = new MaxAccumulator<TSample>(accumulatorBatchSize);

  fRightMaxBuffer = new CircularBuffer<TSample>(fLeftMaxBuffer->getSize());
  fRightMaxBuffer->init(0);
  fRightMaxAccumulator = new MaxAccumulator<TSample>(accumulatorBatchSize);

  fZoomWindow = new ZoomWindow(MAX_ARRAY_SIZE, fLeftMaxBuffer->getSize());
  fZoomWindow->setZoomFactor(DEFAULT_ZOOM_FACTOR_X);

  DLOG_F(INFO, "fMaxBufferSize=%d,accumulatorBatchSize=%d", fLeftMaxBuffer->getSize(), accumulatorBatchSize);

//  if(true)
//  {
//    int expectedDisplayValue = 1;
//    for(int i = 0; i < 250; i++)
//    {
//      auto sample = fromDisplayValue(expectedDisplayValue, 118.0);
//      fLeftMaxBuffer->setAt(0, sample);
//      fLeftMaxBuffer->incrementHead();
//
//      expectedDisplayValue++;
//      if(expectedDisplayValue == 119)
//        expectedDisplayValue = 1;
//    }
//  }

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
// VAC6Processor::genericProcessChannel
/////////////////////////////////////////
template<typename SampleType>
bool VAC6Processor::genericProcessChannel(typename AudioBuffers<SampleType>::Channel const &iIn,
                                          typename AudioBuffers<SampleType>::Channel &iOut,
                                          MaxAccumulator<TSample> *iMaxAccumulator,
                                          CircularBuffer<TSample> *iMaxBuffer)
{
  DCHECK_EQ_F(iIn.getNumSamples(), iOut.getNumSamples());

  bool silent = true;

  auto numSamples = iIn.getNumSamples();
  auto inPtr = iIn.getBuffer();
  auto outPtr = iOut.getBuffer();

  for(int i = 0; i < numSamples; ++i, inPtr++, outPtr++)
  {
    auto sample = *inPtr;

    TSample max;
    if(iMaxAccumulator->accumulate(sample, max))
    {
      iMaxBuffer->setAt(0, max);
      iMaxBuffer->incrementHead();
    }

    if(silent && !pongasoft::VST::Common::isSilent(sample))
      silent = false;

    *outPtr = sample;
  }

  iOut.setSilenceFlag(silent);

  return silent;
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

  auto leftChannel = out.getLeftChannel();
  auto rightChannel = out.getRightChannel();

  genericProcessChannel<SampleType>(in.getLeftChannel(), leftChannel, fLeftMaxAccumulator, fLeftMaxBuffer);
  genericProcessChannel<SampleType>(in.getRightChannel(), rightChannel, fRightMaxAccumulator, fRightMaxBuffer);

  if(fPreviousState.fZoomFactorX != fState.fZoomFactorX)
    fZoomWindow->setZoomFactor(fState.fZoomFactorX, MAX_INPUT_PAGE_OFFSET, *fLeftMaxBuffer);

  // TODO
  // TODO should not assume data.numSamples is of fixed amount... use BATCH_SIZE_IN_MS instead
  // TODO
  auto max = out.absoluteMax();

  auto maxLevelValue = std::max(static_cast<TSample>(max), fMaxLevel.fValue);
  auto maxLevelState = toMaxLevelState(maxLevelValue);

  fMaxLevel.fValue = maxLevelValue;
  fMaxLevel.fState = maxLevelState;

  if(fRateLimiter.shouldUpdate(data.numSamples))
  {
    LCDData lcdData{};
    fZoomWindow->computeZoomWindow(*fLeftMaxBuffer, lcdData.fLeftSamples);
    fZoomWindow->computeZoomWindow(*fRightMaxBuffer, lcdData.fRightSamples);
    lcdData.fSoftClippingLevel = fState.fSoftClippingLevel;

    fMaxLevelUpdate.push(fMaxLevel);
    fLCDDataUpdate.push(lcdData);

    fMaxLevel.fValue = 0;
    fMaxLevel.fState = kStateOk;
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
            newState.fSoftClippingLevel = SoftClippingLevel::fromNormalizedParam(value);
            stateChanged = true;
            break;

          case kMaxLevelReset:
            fMaxLevelResetRequested = value == 1.0;
            break;

          case kLCDZoomFactorX:
            newState.fZoomFactorX = value;
            stateChanged = true;
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

  double savedParam = 0;
  if(!streamer.readDouble(savedParam))
    savedParam = DEFAULT_SOFT_CLIPPING_LEVEL;
  newState.fSoftClippingLevel = SoftClippingLevel{savedParam};

  savedParam = 0;
  if(!streamer.readDouble(savedParam))
    savedParam = DEFAULT_ZOOM_FACTOR_X;
  newState.fZoomFactorX = savedParam;

  fStateUpdate.push(newState);

  DLOG_F(INFO, "VAC6Processor::setState => fSoftClippingLevel=%f, fZoomFactorX=%f",
         newState.fSoftClippingLevel.getValueInSample(),
         newState.fZoomFactorX);

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

  DLOG_F(INFO, "VAC6Processor::getState => fSoftClippingLevel=%f, fZoomFactorX=%f",
         latestState.fSoftClippingLevel.getValueInSample(),
         latestState.fZoomFactorX);

  return kResultOk;
}

///////////////////////////////////
// VAC6Processor::toMaxLevelState
///////////////////////////////////
template<typename SampleType>
EMaxLevelState VAC6Processor::toMaxLevelState(SampleType value)
{
  if(value < MIN_AUDIO_SAMPLE)
    return kStateOk;

  // hard clipping
  if(value > HARD_CLIPPING_LEVEL)
    return kStateHardClipping;

  // soft clipping
  if(value > fState.fSoftClippingLevel.getValueInSample())
    return kStateSoftClipping;

  return kStateOk;

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
      m.setFloat(MAX_LEVEL_VALUE_ATTR, maxLevel.fValue);
      m.setInt(MAX_LEVEL_STATE_ATTR, maxLevel.fState);

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
      m.setBinary(LCDDATA_LEFT_SAMPLES_ATTR, lcdData.fLeftSamples, MAX_ARRAY_SIZE);
      m.setBinary(LCDDATA_RIGHT_SAMPLES_ATTR, lcdData.fRightSamples, MAX_ARRAY_SIZE);
      m.setFloat(LCDDATA_SOFT_CLIPPING_LEVEL_ATTR, fState.fSoftClippingLevel.getValueInSample());

      sendMessage(message);
    }
  }
}


}
}