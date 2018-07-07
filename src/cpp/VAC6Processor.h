#pragma once

#include <public.sdk/source/vst/vstaudioeffect.h>
#include "VAC6Constants.h"
#include "logging/loguru.hpp"
#include "VAC6Model.h"
#include "AudioBuffer.h"
#include "CircularBuffer.h"
#include "ZoomWindow.h"
#include "Concurrent.h"
#include "Clock.h"
#include "VAC6AudioChannelProcessor.h"
#include <base/source/timer.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg;
using namespace Steinberg::Vst;

using namespace VST::Common;

/**
 * class VAC6Processor, main processor for VAC6 VST
 */
class VAC6Processor : public AudioEffect, ITimerCallback
{
public:
  VAC6Processor();

  ~VAC6Processor() override;

  /** Called at first after constructor */
  tresult PLUGIN_API initialize(FUnknown *context) override;

  /** Called at the end before destructor */
  tresult PLUGIN_API terminate() override;

  /** Switch the Plug-in on/off */
  tresult PLUGIN_API setActive(TBool state) override;

  /** Here we go...the process call */
  tresult PLUGIN_API process(ProcessData &data) override;

  /** Asks if a given sample size is supported see \ref SymbolicSampleSizes. */
  tresult PLUGIN_API canProcessSampleSize(int32 symbolicSampleSize) override;

  /** Restore the state (ex: after loading preset or project) */
  tresult PLUGIN_API setState(IBStream *state) override;

  /** Called to save the state (before saving a preset or project) */
  tresult PLUGIN_API getState(IBStream *state) override;

  //--- ---------------------------------------------------------------------
  // create function required for Plug-in factory,
  // it will be called to create new instances of this Plug-in
  //--- ---------------------------------------------------------------------
  static FUnknown *createInstance(void * /*context*/)
  { return (IAudioProcessor *) new VAC6Processor(); }

  tresult PLUGIN_API setupProcessing(ProcessSetup &setup) override;

protected:
  /**
   * Processes the parameters that have changed since the last call to process
   *
   * @param inputParameterChanges
   */
  bool processParameters(IParameterChanges &inputParameterChanges);

  /**
   * Processes inputs (step 2 always called after processing the parameters)
   */
  tresult processInputs(ProcessData &data);

  /**
   * Processes inputs (step 2 always called after processing the parameters)
   */
  template<typename SampleType>
  tresult genericProcessInputs(ProcessData &data);

  // from ITimerCallback
  void onTimer(Timer *timer) override;

protected:
  /**
   * @return the duration of the window in milliseconds
   */
  long getWindowSizeInMillis()
  {
    return fClock.getTimeForSampleCount(fZoomWindow->getVisibleWindowSizeInSamples() * fMaxAccumulatorBatchSize);
  }

private:
  struct State
  {
    SoftClippingLevel fSoftClippingLevel{DEFAULT_SOFT_CLIPPING_LEVEL}; // TODO migrate to UI (not used in processing)
    double fZoomFactorX{DEFAULT_ZOOM_FACTOR_X};
    bool fLeftChannelOn{true};
    bool fRightChannelOn{true};
    bool fLCDLiveView{true};
    int fLCDInputX{MAX_LCD_INPUT_X};
    double fLCDHistoryOffset{MAX_HISTORY_OFFSET};
    Gain fGain1{};
    Gain fGain2{};

    void updateLCDInputX(ProcessData& iData, int iLCDInputX);
    void updateLCDHistoryOffset(ProcessData& iData, double iLCDHistoryOffset);
  };

  bool fMaxLevelResetRequested;

  State fState;
  State fPreviousState;

  Gain fGain;

  SampleRateBasedClock fClock;

  SpinLock::SingleElementQueue<State> fStateUpdate;
  SpinLock::AtomicValue<State> fLatestState;

  uint32 fMaxAccumulatorBatchSize;
  ZoomWindow *fZoomWindow;

  VAC6AudioChannelProcessor *fLeftChannelProcessor;
  VAC6AudioChannelProcessor *fRightChannelProcessor;

  Timer *fTimer;
  SampleRateBasedClock::RateLimiter fRateLimiter;
  SpinLock::SingleElementQueue<LCDData> fLCDDataUpdate;

};

}
}
}