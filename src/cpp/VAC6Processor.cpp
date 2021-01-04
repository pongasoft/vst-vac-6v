#include <base/source/fstreamer.h>
#include <public.sdk/source/vst/vstaudioprocessoralgo.h>

#include <pongasoft/VST/Messaging.h>
#include <pongasoft/VST/Debug/ParamDisplay.h>
#include <pongasoft/VST/Debug/ParamTable.h>

#include "version.h"
#include "jamba_version.h"

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

  for(int i = 0; i < numSamples; ++i)
  {
    TSample sample = inPtr ? *inPtr++ : 0;

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

    if(silent && !pongasoft::VST::isSilent(sample))
      silent = false;

    if(outPtr)
      *outPtr++ = sample;
  }

  iOut.setSilenceFlag(silent);

  return silent;
}

///////////////////////////////////////////
// VAC6Processor::VAC6Processor
///////////////////////////////////////////
VAC6Processor::VAC6Processor() :
  RTProcessor(VAC6ControllerUID),
  fParameters{},
  fState{fParameters},
  fGain{fState.fGain1->getValue() * fState.fGain2->getValue(), DEFAULT_GAIN_FILTER},
  fClock{44100},
  fMaxAccumulatorBatchSize{fClock.getSampleCountFor(ACCUMULATOR_BATCH_SIZE_IN_MS)},
  fZoomWindow{nullptr},
  fLeftChannelProcessor{nullptr},
  fRightChannelProcessor{nullptr},
  fRateLimiter{}
{
  DLOG_F(INFO, "[%s] VAC6Processor() - jamba: %s - plugin: v%s (%s)",
         stringPluginName,
         JAMBA_GIT_VERSION_STR,
         FULL_VERSION_STR,
         BUILD_ARCHIVE_ARCHITECTURE);

#ifndef NDEBUG
  DLOG_F(INFO, "Parameters ---> \n%s", Debug::ParamTable::from(fParameters).full().toString().c_str());
#endif
}

///////////////////////////////////////////
// VAC6Processor::~VAC6Processor
///////////////////////////////////////////
VAC6Processor::~VAC6Processor()
{
  DLOG_F(INFO, "~VAC6Processor()");

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

  tresult result = RTProcessor::initialize(context);
  if(result != kResultOk)
  {
    return result;
  }

  addAudioInput(STR16 ("Stereo In"), SpeakerArr::kStereo);
  addAudioOutput(STR16 ("Stereo Out"), SpeakerArr::kStereo);

#ifndef NDEBUG
  using Key = Debug::ParamDisplay::Key;
  DLOG_F(INFO, "RT Save State - Version=%d --->\n%s",
         fParameters.getRTSaveStateOrder().fVersion,
         Debug::ParamTable::from(getRTState(), true).keys({Key::kID, Key::kTitle}).full().toString().c_str());
#endif

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
  tresult result = RTProcessor::setupProcessing(setup);

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
         "VAC6Processor::setupProcessing(%s, %s, maxSamples=%d, sampleRate=%f, %dms=%d samples)",
         setup.processMode == kRealtime ? "Realtime" : (setup.processMode == kPrefetch ? "Prefetch" : "Offline"),
         setup.symbolicSampleSize == kSample32 ? "32bits" : "64bits",
         setup.maxSamplesPerBlock,
         setup.sampleRate,
         ACCUMULATOR_BATCH_SIZE_IN_MS,
         fMaxAccumulatorBatchSize);

  return result;
}

/////////////////////////////////////////
// VAC6Processor::genericProcessInputs
/////////////////////////////////////////
template<typename SampleType>
tresult VAC6Processor::genericProcessInputs(ProcessData &data)
{
  if(data.numInputs == 0 || data.numOutputs == 0)
  {
    // nothing to do
    return kResultOk;
  }

  AudioBuffers<SampleType> in(data.inputs[0], data.numSamples);
  AudioBuffers<SampleType> out(data.outputs[0], data.numSamples);

  // Handling mono or stereo only
  if(in.getNumChannels() < 1 || in.getNumChannels() > 2 || out.getNumChannels() < 1 || out.getNumChannels() > 2)
    return kResultFalse;

  bool isNewLiveView = false;
  bool isNewPause = false;

  // some DAW like Maschine exposes the controls which then bypasses pause => force into pause
  if(fState.fLCDInputX.hasChanged() || fState.fLCDHistoryOffset.hasChanged())
  {
    if(*fState.fLCDLiveView)
    {
      fState.fLCDLiveView.update(false, data);
    }
  }

  // live view/pause has changed
  if(fState.fLCDLiveView.hasChanged())
  {
    fLeftChannelProcessor->setIsLiveView(*fState.fLCDLiveView);
    fRightChannelProcessor->setIsLiveView(*fState.fLCDLiveView);

    isNewLiveView = *fState.fLCDLiveView;
    isNewPause =!isNewLiveView;
  }

  // Gain filter has changed
  if(fState.fGainFilter.hasChanged())
  {
    fGain.setFilterOn(*fState.fGainFilter);
  }

    // gain has changed
  if(fState.fGain1.hasChanged() || fState.fGain2.hasChanged())
  {
    // simply combine the 2 gains
    fGain.setTargetValue(fState.fGain1->getValue() * fState.fGain2->getValue());
  }

  // Zoom has changed
  if(fState.fZoomFactorX.hasChanged())
  {
    if(*fState.fLCDLiveView)
    {
      fZoomWindow->setZoomFactor(*fState.fZoomFactorX);
    }
    else
    {
      int newLCDInputX =
        fZoomWindow->setZoomFactor(*fState.fZoomFactorX,
                                   fState.fLCDInputX != LCD_INPUT_X_NOTHING_SELECTED ? *fState.fLCDInputX : MAX_ARRAY_SIZE / 2,
                                   { *fState.fLeftChannelOn ? &fLeftChannelProcessor->getMaxBuffer() : nullptr,
                                     *fState.fRightChannelOn ? &fRightChannelProcessor->getMaxBuffer() : nullptr });

      if(fState.fLCDInputX != LCD_INPUT_X_NOTHING_SELECTED && fState.fLCDInputX != newLCDInputX)
      {
        fState.fLCDInputX.update(newLCDInputX, data);
      }

      double newLCDHistoryOffset = fZoomWindow->getWindowOffset();

      if(newLCDHistoryOffset != fState.fLCDHistoryOffset)
      {
        fState.fLCDHistoryOffset.update(newLCDHistoryOffset, data);
      }
    }

    fLeftChannelProcessor->setDirty();
    fRightChannelProcessor->setDirty();
  }

  // Scrollbar has been moved
  if(fState.fLCDHistoryOffset.hasChanged())
  {
    fZoomWindow->setWindowOffset(*fState.fLCDHistoryOffset);
    fLeftChannelProcessor->setDirty();
    fRightChannelProcessor->setDirty();
  }

  // after we cancel pause we need to reset LCDInputX and LCDHistoryOffset
  if(isNewLiveView)
  {
    if(fState.fLCDInputX != LCD_INPUT_X_NOTHING_SELECTED)
    {
      fState.fLCDInputX.update(LCD_INPUT_X_NOTHING_SELECTED, data);
    }

    if(fState.fLCDHistoryOffset != MAX_HISTORY_OFFSET)
    {
      fState.fLCDHistoryOffset.update(MAX_HISTORY_OFFSET, data);
      fZoomWindow->setWindowOffset(*fState.fLCDHistoryOffset);
      fLeftChannelProcessor->setDirty();
      fRightChannelProcessor->setDirty();
    }
  }

  // we need to adjust the filtered gain
  fGain.adjust();
  auto gain = *fState.fBypass ? Gain::Unity : fGain.getValue();

  // in mono case there could be only one channel
  auto leftChannel = out.getLeftChannel();
  fLeftChannelProcessor->genericProcessChannel<SampleType>(fZoomWindow, in.getLeftChannel(), leftChannel, gain);

  if(in.getNumChannels() == 2)
  {
    auto rightChannel = out.getNumChannels() == 2 ? out.getRightChannel() : leftChannel;
    fRightChannelProcessor->genericProcessChannel<SampleType>(fZoomWindow, in.getRightChannel(), rightChannel, gain);
  }

  // if reset of max level is requested (pressing momentary button) then we need to reset the accumulator
  if(*fState.fMaxLevelReset)
  {
    fLeftChannelProcessor->resetMaxLevelSinceReset();
    fRightChannelProcessor->resetMaxLevelSinceReset();
  }

  // is it time to update the UI?
  if(isNewPause || fRateLimiter.shouldUpdate(static_cast<uint32>(data.numSamples)))
  {
    fState.fHistoryData.broadcast([this](HistoryData *oHistoryData) {

      LCDData &lcdData = oHistoryData->fLCDData;

      // left
      if(*fState.fLeftChannelOn)
      {
        fLeftChannelProcessor->computeZoomSamples(MAX_ARRAY_SIZE, lcdData.fLeftChannel.fSamples);
        lcdData.fLeftChannel.fMaxLevelSinceReset = fLeftChannelProcessor->getMaxLevelSinceReset();
      }
      lcdData.fLeftChannel.fOn = *fState.fLeftChannelOn;


      // right
      if(*fState.fRightChannelOn)
      {
        fRightChannelProcessor->computeZoomSamples(MAX_ARRAY_SIZE, lcdData.fRightChannel.fSamples);
        lcdData.fRightChannel.fMaxLevelSinceReset = fRightChannelProcessor->getMaxLevelSinceReset();
      }
      lcdData.fRightChannel.fOn = *fState.fRightChannelOn;

    });
  }

  return kResultOk;
}

}
}