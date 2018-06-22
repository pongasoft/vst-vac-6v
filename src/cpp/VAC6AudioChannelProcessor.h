#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <algorithm>
#include "CircularBuffer.h"
#include "VAC6Constants.h"
#include "VAC6Model.h"
#include "Clock.h"
#include "AudioBuffer.h"
#include "ZoomWindow.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg;
using namespace Steinberg::Vst;

using namespace VST::Common;

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

class VAC6AudioChannelProcessor
{
public:
  explicit VAC6AudioChannelProcessor(SampleRateBasedClock const &iClock);

  ~VAC6AudioChannelProcessor();

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
    fMaxLevel = 0;
  }

  void resetMaxLevelAccumulator(long iMaxLevelResetInSeconds)
  {
    // if 0 we still do the same buffering as the other buffer otherwise it would not match
    long maxLevelResetMS = iMaxLevelResetInSeconds == 0 ? ACCUMULATOR_BATCH_SIZE_IN_MS : iMaxLevelResetInSeconds * 1000;
    fMaxLevelAccumulator.reset(fClock.getSampleCountFor(maxLevelResetMS));
    fMaxLevel = 0;
  }

  TSample getMaxLevel() const
  {
    return fMaxLevel;
  }

  /**
   * @param iZoomFactorPercent zoom factor between 0-1 (where 1 is min zoom, and 0 is max zoom)
   */
  void setZoomFactor(double iZoomFactorPercent)
  {
    fZoomWindow.setZoomFactor(iZoomFactorPercent);
    fZoomMaxAccumulator = fZoomWindow.computeZoomWindow(*fMaxBuffer, *fZoomMaxBuffer);
  }

  /**
   * @return the duration of the window in milliseconds
   */
  long getWindowSizeInMillis()
  {
//    DLOG_F(INFO, "getWindowSizeInMillis - %d,%ld",
//           fZoomWindow.getVisibleWindowSizeInSamples(),
//           fClock.getTimeForSampleCount(fZoomWindow.getVisibleWindowSizeInSamples()));
//
    return fClock.getTimeForSampleCount(fZoomWindow.getVisibleWindowSizeInSamples() * fMaxAccumulatorForBuffer.getBatchSize());
  }
  /**
   * Copy the zoomed samples into the array provided.
   *
   * @param iNumSamples size of the provided array
   */
  void computeZoomSamples(int iNumSamples, TSample *oSamples) const;

  template<typename SampleType>
  bool genericProcessChannel(const typename AudioBuffers<SampleType>::Channel &iIn, typename AudioBuffers<SampleType>::Channel &iOut);

private:
  SampleRateBasedClock fClock;

  MaxAccumulator fMaxAccumulatorForBuffer;
  CircularBuffer<TSample> *const fMaxBuffer;

  MaxAccumulator fMaxLevelAccumulator;
  TSample fMaxLevel;

  ZoomWindow fZoomWindow;
  TZoom::MaxAccumulator fZoomMaxAccumulator;
  CircularBuffer<TSample> *const fZoomMaxBuffer;
};

}
}
}
