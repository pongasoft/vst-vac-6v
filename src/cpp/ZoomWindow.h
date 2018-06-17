#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <algorithm>
#include "CircularBuffer.h"
#include "logging/loguru.hpp"

namespace pongasoft {
namespace VST {
namespace Common {


using TSample = Steinberg::Vst::Sample64;

/**
 * batchSize is linked to how precise you want to be in regards to the zoom factor
 * For example a batchSize of 10 means that means that we can handle zoom factor like 1.2, 3.4, etc...
 * For example a batchSize of 100 means that means that we can handle zoom factor like 1.24, 3.49, etc... */
template <int batchSize = 10>
class Zoom
{
public:
  using class_type = Zoom<batchSize>;

  class MaxAccumulator
  {
  public:
    explicit MaxAccumulator(class_type const *iZoom) :
      fZoom{iZoom},
      fAccumulatedMax{0},
      fAccumulatedSamples{0},
      fBatchSizeIdx{0}
    {
    }

    MaxAccumulator(class_type const *iZoom, int iBatchSizeIdx) :
      fZoom{iZoom},
      fAccumulatedMax{0},
      fAccumulatedSamples{0},
      fBatchSizeIdx{iBatchSizeIdx}
    {
    }

    template<typename SampleType>
    bool accumulate(SampleType iSample, SampleType &oMaxSample)
    {
      if(iSample < 0)
        iSample = -iSample;

      fAccumulatedMax = std::max(fAccumulatedMax, iSample);
      fAccumulatedSamples++;

      if(fAccumulatedSamples == fZoom->fBatchSizes[fBatchSizeIdx])
      {
        oMaxSample = fAccumulatedMax;
        fAccumulatedMax = 0;
        fAccumulatedSamples = 0;
        fBatchSizeIdx++;
        if(fBatchSizeIdx == batchSize)
          fBatchSizeIdx = 0;
        return true;
      }

      return false;
    }

  private:
    class_type const *fZoom;

    TSample fAccumulatedMax;
    int fAccumulatedSamples;
    int fBatchSizeIdx;
  };

public:
  friend class class_type::MaxAccumulator;

  explicit Zoom(double iZoomFactor = 1.0)
  {
    setZoomFactor(iZoomFactor);
  }

  /**
   * @param iZoomFactor zoom factor is 1.0 for min zoom. 2.0 for example means twice as big... etc...
   */
  MaxAccumulator setZoomFactor(double iZoomFactor)
  {
    DCHECK_F(iZoomFactor >= 1.0);

    fZoomFactor = static_cast<int>(iZoomFactor * batchSize);
    init();
    return MaxAccumulator{this};
  }

  /**
   * @param iZoomPointIndex negative index on where to start
   * @param oOffset the offset (output) on where the start in the non zoomed buffer
   * @return an accumulator to start accumulating from a given zoom point index
   */
  MaxAccumulator getAccumulatorFromIndex(int iZoomPointIndex, int &oOffset) const;

  /**
   * @return true if there is no zoom at all (1.0)
   */
  inline bool isNoZoom() const
  {
    return fZoomFactor == batchSize;
  }

  /**
   * This number represents the number of zoomed points to fit getTotalBatchSizeInSamples()
   */
  inline int getBatchSize() const
  {
    return batchSize;
  }

  /**
   * @return the total number of samples that "fit" into batchSize
   */
  inline int getBatchSizeInSamples() const
  {
    return fZoomFactor;
  }


private:

  // internally called to initialize the batch size array
  void init();

  friend class ZoomWindow;

  // void reset(TSample iSample, int iPercent);

  // zoom factor as an int
  int fZoomFactor;

  // maintains the number of samples to be taken into consideration for each zoomed point
  int fBatchSizes[batchSize];
  // maintains the offsets at which each zoom point starts
  int fOffset[batchSize];
};

using TZoom = Zoom<10>;

/**
 * Represents a zoom window
 */
class ZoomWindow
{

public:
  ZoomWindow(int iVisibleWindowSize, int iBufferSize);

  /**
   * @param iZoomFactorPercent zoom factor between 0-1 (where 1 is min zoom, and 0 is max zoom)
   */
  TZoom::MaxAccumulator setZoomFactor(double iZoomFactorPercent);

  /**
   * Return the visible size of the window */
  inline int getVisibleWindowSize() const
  {
    return fVisibleWindowSize;
  }

  /**
   * Computes the zoom
   * @param iBuffer
   * @param oBuffer
   */
  TZoom::MaxAccumulator computeZoomWindow(const CircularBuffer <TSample> &iBuffer, CircularBuffer <TSample> &oBuffer) const;

private:
  // here iIdx is relative to the right of the screen (see fWindowOffset)
  TZoom::MaxAccumulator getMaxAccumulatorFromIndex(int iIdx, int &oOffset) const;


  // Convenient method to compute the zoom point at the left of the LCD screen
  TZoom::MaxAccumulator getMaxAccumulatorFromLeftOfScreen(int &oOffset) const;

//
////
////  /**
////   * Computes the zoom value from a zoom point
////   */
////  TSample computeZoomValue(ZoomedBuffer const &iZoomPoint, CircularBuffer<TSample> const &iBuffer) const;
//
//  /**
//   * Find the zoom point so that zp.fBufferOffset is the closest to iBufferOffset and returns
//   * the index. Also tries to find the point where the zoom value would be the closest.
//   */
//  //int findClosestWindowIndex(int iBufferOffset, TSample const &iZoomValue, CircularBuffer<TSample> const &iBuffer) const;
//

  inline int minWindowIdx() const
  {
    return fMinWindowOffset - fVisibleWindowSize + 1;
  }

private:
  int const fVisibleWindowSize;
  int const fBufferSize;

  // offset in the zoomed window (with -1 being the right of the LCD screen for most recent point)
  // this assumes that it corresponds to -1 in fBuffer as well
  int fWindowOffset;

  // the minimum window offset to fit inside the buffer
  int fMinWindowOffset;

  // the maximum amount of zoom allowed
  int fMaxZoomFactor;

  /**
   * Zoom associated to this window */
  TZoom fZoom;
};

////////////////////////////////////////////////////////////
// Zoom::init
////////////////////////////////////////////////////////////
template<int batchSize>
void Zoom<batchSize>::init()
{
  if(isNoZoom())
    return;

  int accumulatedZoom = 0;
  int accumulatedSamples = 0;
  int totalBatchSize = 0;
  for(int i = 0; i < batchSize; i++)
  {
    int newAccumulatedZoom = accumulatedZoom + fZoomFactor;

    while(newAccumulatedZoom >= batchSize)
    {
      accumulatedSamples++;
      newAccumulatedZoom -= batchSize;
    }

    fBatchSizes[i] = accumulatedSamples;
    fOffset[i] = totalBatchSize;
    totalBatchSize += accumulatedSamples;
    accumulatedSamples = 0;

    accumulatedZoom = newAccumulatedZoom;
  }

  DCHECK_EQ_F(totalBatchSize, fZoomFactor);
}

////////////////////////////////////////////////////////////
// Zoom::getAccumulatorFromIndex
////////////////////////////////////////////////////////////
template<int batchSize>
typename Zoom<batchSize>::MaxAccumulator Zoom<batchSize>::getAccumulatorFromIndex(int iZoomPointIndex, int &oOffset) const
{
  // take into account how many multiples of batches we have to skip
  int offset = -getBatchSizeInSamples();
  offset += iZoomPointIndex / batchSize * getBatchSizeInSamples();

  // determine which index to use
  int batchSizeIndex = iZoomPointIndex % batchSize;
  if(batchSizeIndex == 0)
    offset += getBatchSizeInSamples();
  else
    batchSizeIndex += batchSize; // because iIdx is negative, the modulo will be negative as well...

  DCHECK_F(batchSizeIndex >= 0 && batchSizeIndex < batchSize);

  oOffset = offset + fOffset[batchSizeIndex];

  return Zoom<batchSize>::MaxAccumulator(this, batchSizeIndex);
}

}
}
}