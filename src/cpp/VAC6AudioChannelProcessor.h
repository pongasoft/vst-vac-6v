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
  explicit MaxAccumulator(uint32 iBatchSize) : fBatchSize{iBatchSize}
  {
  }

  inline uint32 getBatchSize() const
  {
    return fBatchSize;
  }

  inline TSample getAccumulatedMax() const
  {
    return fAccumulatedMax;
  }

  /**
   * A batch size set to 0 means that it will always accumulate so "accumulate" will always return false =>
   * you use getAccumulatedMax() to get the most recent accumulated value
   */
  void reset(uint32 iBatchSize)
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

    if(fBatchSize > 0)
    {
      fAccumulatedSamples++;

      if(fAccumulatedSamples == fBatchSize)
      {
        oMaxSample = fAccumulatedMax;
        fAccumulatedMax = 0;
        fAccumulatedSamples = 0;
        return true;
      }
    }

    return false;
  }

private:
  uint32 fBatchSize;

  TSample fAccumulatedMax{0};
  uint32 fAccumulatedSamples{0};
};

class VAC6AudioChannelProcessor
{
public:
  // Constructor
  explicit VAC6AudioChannelProcessor(SampleRateBasedClock const &iClock);

  // Destructor
  ~VAC6AudioChannelProcessor();

  // getMaxBuffer
  CircularBuffer<TSample> const &getMaxBuffer() const
  {
    return *fMaxBuffer;
  };

  // getBufferAccumulatorBatchSize
  long getBufferAccumulatorBatchSize() const
  {
    return fMaxAccumulatorForBuffer.getBatchSize();
  }

  // resetMaxLevelAccumulator
  void resetMaxLevelAccumulator()
  {
    fMaxLevelAccumulator.reset();
    fMaxLevel = 0;
  }

  // resetMaxLevelAccumulator
  void resetMaxLevelAccumulator(uint32 iMaxLevelResetInSeconds)
  {
    if(iMaxLevelResetInSeconds == 0)
      fMaxLevelAccumulator.reset(0);
    else
    {
      // if 0 we still do the same buffering as the other buffer otherwise it would not match
      fMaxLevelAccumulator.reset(fClock.getSampleCountFor(iMaxLevelResetInSeconds * 1000));
    }

    if(fIsLiveView)
      fMaxLevel = 0;
  }

  // getMaxLevel
  TSample getMaxLevel() const
  {
    return fMaxLevel;
  }

  /**
   * @param iZoomFactorPercent zoom factor between 0-1 (where 1 is min zoom, and 0 is max zoom)
   */
  void setZoomFactor(double iZoomFactorPercent);

  /**
   * Called in pause mode to make sure that the zoom happens around the correct point
   * @param iZoomFactorPercent zoom factor between 0-1 (where 1 is min zoom, and 0 is max zoom)
   */
  void setZoomFactor(double iZoomFactorPercent, int iLCDInputX);

  // setIsLiveView
  void setIsLiveView(bool iIsLiveView);

  // setLCDInputX
  void setLCDInputX(int iLCDInputX);

  // setHistoryOffset
  void setHistoryOffset(double iHistoryOffset);

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
  bool genericProcessChannel(const typename AudioBuffers<SampleType>::Channel &iIn,
                             typename AudioBuffers<SampleType>::Channel &iOut);

protected:
  // setPausedZoomMaxBufferOffset
  void setPausedZoomMaxBufferOffset(int iOffset);

private:
  SampleRateBasedClock fClock;

  MaxAccumulator fMaxAccumulatorForBuffer;
  CircularBuffer<TSample> *const fMaxBuffer;

  MaxAccumulator fMaxLevelAccumulator;
  TSample fMaxLevel;

  ZoomWindow fZoomWindow;
  TZoom::MaxAccumulator fZoomMaxAccumulator;
  CircularBuffer<TSample> *const fZoomMaxBuffer;
  bool fNeedToRecomputeZoomMaxBuffer;

  bool fIsLiveView;
  int fPausedZoomMaxBufferOffset;
};

}
}
}
