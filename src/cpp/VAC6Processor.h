#pragma once

#include <public.sdk/source/vst/vstaudioeffect.h>
#include "VAC6Constants.h"
#include "logging/loguru.hpp"
#include "VAC6Model.h"
#include "CircularBuffer.h"
#include "ZoomWindow.h"
#include "Concurrent.h"
#include <base/source/timer.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg;
using namespace Steinberg::Vst;

using namespace VST::Common;
using namespace pongasoft::Common;

/**
 * Keeps track of the time in number of samples processed vs sample rate
 */
class RateLimiter
{
public:
  RateLimiter() : fRateLimitInSamples{0}, fSampleCount{0}
  {}

  void init(SampleRate sampleRate, long rateLimitInMillis)
  {
    fRateLimitInSamples = static_cast<long>(rateLimitInMillis * sampleRate / 1000);
    fSampleCount = 0;
  }

  bool shouldUpdate(int numSamples)
  {
    fSampleCount += numSamples;
    if(fSampleCount >= fRateLimitInSamples)
    {
      fSampleCount -= fRateLimitInSamples;
      return true;
    }
    return false;
  }

private:
  long fRateLimitInSamples;
  long fSampleCount;
};

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

  template<typename SampleType>
  inline EMaxLevelState toMaxLevelState(SampleType value);

  // from ITimerCallback
  void onTimer(Timer *timer) override;

private:
  struct State
  {
    SoftClippingLevel fSoftClippingLevel{DEFAULT_SOFT_CLIPPING_LEVEL};
    double fZoomFactorX{DEFAULT_ZOOM_FACTOR_X};
  };

  MaxLevel fMaxLevel;
  bool fMaxLevelResetRequested;

  State fState;
  State fPreviousState;

  SingleElementQueue<State> fStateUpdate;
  AtomicValue<State> fLatestState;

  CircularBuffer<TSample> *fMaxBuffer;
  ZoomWindow *fZoomWindow;

  Timer *fTimer;
  RateLimiter fRateLimiter;
  SingleElementQueue<MaxLevel> fMaxLevelUpdate;
  SingleElementQueue<LCDData> fLCDDataUpdate;
};

}
}
}