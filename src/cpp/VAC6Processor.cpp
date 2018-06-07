#include <base/source/fstreamer.h>
#include <public.sdk/source/vst/vstaudioprocessoralgo.h>

#include "AudioBuffer.h"
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
  fSoftClippingLevel{},
  fTimer{nullptr}
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

  fRateLimiter.init(setup.sampleRate, UI_FRAME_RATE_MS);

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
  // 1. process parameter changes
  if(data.inputParameterChanges != nullptr)
    processParameters(*data.inputParameterChanges);

  // 2. process inputs
  tresult res = processInputs(data);

  // 3. update the state
  // TODO

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

  tresult res = out.copyFrom(in);

  auto maxLevelValue = std::max(static_cast<Sample64>(out.absoluteMax()), fMaxLevel.fValue);
  auto maxLevelState = toMaxLevelState(maxLevelValue);

  fMaxLevel.fValue = maxLevelValue;
  fMaxLevel.fState = maxLevelState;

  out.adjustSilenceFlags();

  if(fRateLimiter.shouldUpdate(data.numSamples))
  {
    fMaxLevelValue.save(fMaxLevel);

    fMaxLevel.fValue = 0;
    fMaxLevel.fState = kStateOk;
  }

  return res;
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
void VAC6Processor::processParameters(IParameterChanges &inputParameterChanges)
{
  int32 numParamsChanged = inputParameterChanges.getParameterCount();
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
            fSoftClippingLevel = SoftClippingLevel::fromNormalizedParam(value);
            break;

          default:
            // shouldn't happen?
            break;
        }
      }
    }
  }
}

///////////////////////////////////
// VAC6Processor::setState
///////////////////////////////////
tresult VAC6Processor::setState(IBStream *state)
{
  if(state == nullptr)
    return kResultFalse;

  IBStreamer streamer(state, kLittleEndian);

  double savedParam = 0;
  if(!streamer.readDouble(savedParam))
    return kResultFalse;

  fSoftClippingLevel = savedParam;

  DLOG_F(INFO, "VAC6Processor::setState => fSoftClippingLevel=%f", savedParam);

  return kResultOk;
}

///////////////////////////////////
// VAC6Processor::getState
///////////////////////////////////
tresult VAC6Processor::getState(IBStream *state)
{
  if(state == nullptr)
    return kResultFalse;

  IBStreamer streamer(state, kLittleEndian);

  streamer.writeDouble(fSoftClippingLevel.getValueInSample());

  DLOG_F(INFO, "VAC6Processor::getState => fSoftClippingLevel=%f", fSoftClippingLevel.getValueInSample());

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
  if(value > 1.0)
    return kStateHardClipping;

  // soft clipping
  if(value > fSoftClippingLevel.getValueInSample())
    return kStateSoftClipping;

  return kStateOk;

}

///////////////////////////////////////////
// VAC6Processor::onTimer
///////////////////////////////////////////
void VAC6Processor::onTimer(Timer * /* timer */)
{
  MaxLevel maxLevel{};
  if(fMaxLevelValue.load(maxLevel))
  {
    IMessage *message = allocateMessage();
    if(!message)
      return;

    OPtr<IMessage> msgReleaser = message;

    Message m{message};

    m.setMessageID(kMaxLevel_MID);
    m.setFloat("Value", maxLevel.fValue);
    m.setInt("State", maxLevel.fState);

    sendMessage(message);
  }
}


}
}