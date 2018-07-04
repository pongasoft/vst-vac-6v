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
  explicit VAC6AudioChannelProcessor(const SampleRateBasedClock &iClock,
                                     ZoomWindow *iZoomWindow,
                                     uint32 iMaxAccumulatorBatchSize,
                                     int iMaxBufferSize);

  // Destructor
  ~VAC6AudioChannelProcessor();

  // getMaxBuffer
  CircularBuffer<TSample> const &getMaxBuffer() const
  {
    return *fMaxBuffer;
  };

  // resetMaxLevelSinceLastReset
  void resetMaxLevelSinceLastReset()
  {
    fMaxLevelSinceLastReset = -1;
  }

  // getMaxLevelSinceLastReset
  TSample getMaxLevelSinceLastReset() const
  {
    return fMaxLevelSinceLastReset;
  }

  /**
   * Mark the channel processor dirty in order to recompute the max zoom buffer
   */
  void setDirty();

  // setIsLiveView
  void setIsLiveView(bool iIsLiveView);

  /**
   * Copy the zoomed samples into the array provided.
   *
   * @param iNumSamples size of the provided array
   */
  void computeZoomSamples(int iNumSamples, TSample *oSamples) const;

  template<typename SampleType>
  bool genericProcessChannel(ZoomWindow const *iZoomWindow,
                             const typename AudioBuffers<SampleType>::Channel &iIn,
                             typename AudioBuffers<SampleType>::Channel &iOut);

private:
  SampleRateBasedClock fClock;

  MaxAccumulator fMaxAccumulatorForBuffer;
  CircularBuffer<TSample> *const fMaxBuffer;

  TSample fMaxLevelSinceLastReset;

  TZoom::MaxAccumulator fZoomMaxAccumulator;
  CircularBuffer<TSample> *const fZoomMaxBuffer;
  bool fNeedToRecomputeZoomMaxBuffer;

  bool fIsLiveView;
};

}
}
}
