#pragma once

#include <base/source/timer.h>
#include <public.sdk/source/vst/vstaudioeffect.h>
#include <pongasoft/Utils/Concurrent/Concurrent.h>
#include <pongasoft/Utils/Collection/CircularBuffer.h>
#include <pongasoft/VST/SampleRateBasedClock.h>
#include <pongasoft/VST/AudioBuffer.h>
#include <pongasoft/logging/loguru.hpp>
#include <pongasoft/VST/RT/RTProcessor.h>
#include "VAC6Constants.h"
#include "VAC6Model.h"
#include "ZoomWindow.h"
#include "VAC6AudioChannelProcessor.h"
#include "VAC6Plugin.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg;
using namespace Steinberg::Vst;

using namespace pongasoft::Utils;

/**
 * class VAC6Processor, main processor for VAC6 VST
 */
class VAC6Processor : public RT::RTProcessor
{
public:
  //--- ---------------------------------------------------------------------
  // create function required for Plug-in factory,
  // it will be called to create new instances of this Plug-in
  //--- ---------------------------------------------------------------------
  static FUnknown *createInstance(void * /*context*/) { return (IAudioProcessor *) new VAC6Processor(); }

public:
  // Constructor
  VAC6Processor();

  // Destructor
  ~VAC6Processor() override;

  // getRTState
  RTState *getRTState() override { return &fState; }

  /** Called at first after constructor (setup input/output) */
  tresult PLUGIN_API initialize(FUnknown *context) override;

  /** Called at the end before destructor */
  tresult PLUGIN_API terminate() override;

  // This is where the setup happens which depends on sample rate, etc..
  tresult PLUGIN_API setupProcessing(ProcessSetup &setup) override;

protected:
  /**
   * Processes inputs (step 2 always called after processing the parameters)
   */
  template<typename SampleType>
  tresult genericProcessInputs(ProcessData &data);

  // processInputs32Bits
  tresult processInputs32Bits(ProcessData &data) override { return genericProcessInputs<Sample32>(data); }

  // processInputs64Bits
  tresult processInputs64Bits(ProcessData &data) override { return genericProcessInputs<Sample64>(data); }

  // onGUITimer
  void onGUITimer() override;

private:
  VAC6Parameters fParameters;
  VAC6RTState fState;

  FilteredGain fGain;

  SampleRateBasedClock fClock;

  uint32 fMaxAccumulatorBatchSize;
  ZoomWindow *fZoomWindow;

  VAC6AudioChannelProcessor *fLeftChannelProcessor;
  VAC6AudioChannelProcessor *fRightChannelProcessor;

  SampleRateBasedClock::RateLimiter fRateLimiter;
  Concurrent::WithSpinLock::SingleElementQueue<LCDData> fLCDDataUpdate;
};

}
}
}