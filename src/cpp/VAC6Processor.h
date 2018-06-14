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
#include <base/source/timer.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg;
using namespace Steinberg::Vst;

using namespace VST::Common;
using namespace pongasoft::Common;

/**
 * Processor for a single audio channel
 */
class AudioChannelProcessor
{
private:
  class MaxAccumulator
  {
  public:
    explicit MaxAccumulator(long iBatchSize) : fBatchSize{iBatchSize}
    {
    }

    inline long getBatchSize() const
    {
      return fBatchSize;
    }

    inline TSample getAccumulatedMax() const
    {
      return fAccumulatedMax;
    }

    void reset(long iBatchSize)
    {
      fBatchSize = iBatchSize;
      fAccumulatedMax = 0;
      fAccumulatedSamples = 0;
    }

    void reset()
    {
      reset(fBatchSize);
    }

    template<typename SampleType>
    bool accumulate(SampleType iSample, SampleType &oMaxSample)
    {
      if(iSample < 0)
        iSample = -iSample;

      fAccumulatedMax = std::max(fAccumulatedMax, iSample);
      fAccumulatedSamples++;

      if(fAccumulatedSamples == fBatchSize)
      {
        oMaxSample = fAccumulatedMax;
        fAccumulatedMax = 0;
        fAccumulatedSamples = 0;
        return true;
      }

      return false;
    }

  private:
    long fBatchSize;

    TSample fAccumulatedMax{0};
    int32 fAccumulatedSamples{0};
  };

public:
  explicit AudioChannelProcessor(SampleRateBasedClock const &iClock) :
    fMaxAccumulatorForBuffer(iClock.getSampleCountFor(ACCUMULATOR_BATCH_SIZE_IN_MS)),
    fMaxBuffer{new CircularBuffer<TSample>(
      static_cast<int>(ceil(iClock.getSampleCountFor(HISTORY_SIZE_IN_SECONDS * 1000) / fMaxAccumulatorForBuffer.getBatchSize())))},
    fMaxLevelAccumulator(iClock.getSampleCountFor(DEFAULT_MAX_LEVEL_RESET_IN_SECONDS * 1000)),
    fMaxLevel{-1}
  {
    fMaxBuffer->init(0);
  }

  ~AudioChannelProcessor()
  {
    delete fMaxBuffer;
  }

  CircularBuffer<TSample> const &getMaxBuffer() const
  {
    return *fMaxBuffer;
  };

  long getBufferAccumulatorBatchSize() const
  {
    return fMaxAccumulatorForBuffer.getBatchSize();
  }

  void resetMaxLevelAccumulator()
  {
    fMaxLevelAccumulator.reset();
    fMaxLevel = -1;
  }

  void resetMaxLevelAccumulator(SampleRateBasedClock const &iClock, long iMaxLevelResetInSeconds)
  {
    // if 0 we still do the same buffering as the other buffer otherwise it would not match
    long maxLevelResetMS = iMaxLevelResetInSeconds == 0 ? ACCUMULATOR_BATCH_SIZE_IN_MS : iMaxLevelResetInSeconds * 1000;
    fMaxLevelAccumulator.reset(iClock.getSampleCountFor(maxLevelResetMS));
    fMaxLevel = -1;
  }

  TSample getMaxLevel() const
  {
    return fMaxLevel;
  }

  template<typename SampleType>
  bool genericProcessChannel(const typename AudioBuffers<SampleType>::Channel &iIn, typename AudioBuffers<SampleType>::Channel &iOut);

private:
  MaxAccumulator fMaxAccumulatorForBuffer;
  CircularBuffer<TSample> *const fMaxBuffer;

  MaxAccumulator fMaxLevelAccumulator;
  TSample fMaxLevel;
};

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

  bool fMaxLevelResetRequested;

  State fState;
  State fPreviousState;

  SampleRateBasedClock fClock;

  SingleElementQueue<State> fStateUpdate;
  AtomicValue<State> fLatestState;

  AudioChannelProcessor *fLeftChannelProcessor;
  AudioChannelProcessor *fRightChannelProcessor;

  ZoomWindow *fZoomWindow;

  Timer *fTimer;
  SampleRateBasedClock::RateLimiter fRateLimiter;
  SingleElementQueue<MaxLevel> fMaxLevelUpdate;
  SingleElementQueue<LCDData> fLCDDataUpdate;

};

}
}
}