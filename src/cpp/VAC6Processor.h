#pragma once

#include <public.sdk/source/vst/vstaudioeffect.h>
#include "VAC6Constants.h"
#include "CircularFIFO.h"
#include "logging/loguru.hpp"
#include <base/source/timer.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg;
using namespace Steinberg::Vst;

/**
 * Keeps track of the time in number of samples processed vs sample rate
 */
class RateLimiter
{
public:
  RateLimiter() : fRateLimitInSamples{0}, fSampleCount{0} {}

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

/**
 * Wraps a single value that can be saved by 1 thread and read by another without using any locks.
 * Uses a CircularFIFO (aka ring buffer) under the cover. The Size parameter type corresponds to the size of the
 * circular FIFO and can be tweaked to account for slow reads. If the reader is slightly slower than the writer,
 * it will be able to catch up because only the last write is returned. If the reader is way too slow, then the writer
 * will not be able to write anymore (in which case the Size parameter should be increased).
 */
template<typename T, size_t Size>
class LockFreeValue
{
  using MessageQueue = Common::MemorySequentialConsistent::CircularFIFO<T, Size>;

public:
  bool save(T const &value)
  {
    bool saved = fMessageQueue.push(value);
    if(!saved)
      DLOG_F(WARNING, "LockFreeValue - could not save value");
    return saved;
  }

  bool load(T &item)
  {
    bool res = fMessageQueue.pop(item);

    // if we were able to pop 1 item, we continue to pop until there is no more to pop => last one will be returned
    if(res)
    {
      while(fMessageQueue.pop(item)) { }
    }

    return res;
  }

private:
  MessageQueue fMessageQueue;
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
  void processParameters(IParameterChanges &inputParameterChanges);

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
  MaxLevel fMaxLevel;

  Timer *fTimer;
  RateLimiter fRateLimiter;
  LockFreeValue<MaxLevel, 3> fMaxLevelValue;
};

}
}
}