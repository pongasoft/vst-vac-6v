#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <algorithm>
#include "CircularBuffer.h"
#include "logging/loguru.hpp"

namespace pongasoft {
namespace VST {
namespace Common {

using namespace Common;

constexpr int ZOOM_BATCH_SIZE = 10;

//const int FLOAT_TO_INT_FACTOR = 10;  // using one digit precision (1.2, 1.3, etc...)

using TSample = Steinberg::Vst::Sample64;


/**
 * batchSize is linked to how precise you want to be in regards to the zoom factor
 * For example a batchSize of 10 means that means that we can handle zoom factor like 1.2, 3.4, etc...
 * For example a batchSize of 100 means that means that we can handle zoom factor like 1.24, 3.49, etc... */
template <int batchSize = 10>
class Zoom
{
  class ZoomedBuffer
  {
  public:

    TSample nextSample();

    inline int getCurrentBufferOffset() const
    {
      return fBufferOffset;
    }

  private:
    friend class Zoom<batchSize>;

    ZoomedBuffer(Zoom<batchSize> const &iZoom, CircularBuffer<TSample> const &iBuffer, int iBufferOffset, int iBatchSizeIdx);

    Zoom<batchSize> const &fZoom;

    CircularBuffer<TSample> const &fBuffer;
    int fBufferOffset;

    TSample fAccumulatedMax;
    int fAccumulatedSamples;
    int fBatchSizeIdx;
  };

public:
  friend class Zoom<batchSize>::ZoomedBuffer;

  explicit Zoom(double iZoomFactor = 1.0)
  {
    setZoomFactor(iZoomFactor);
  }

  /**
   * @param iZoomFactor zoom factor is 1.0 for min zoom. 2.0 for example means twice as big... etc...
   */
  void setZoomFactor(double iZoomFactor);

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

  /**
   * Returns a zoomed version of the buffer starting at iZoomPointIndex
   */
  ZoomedBuffer zoomFromIndex(int iZoomPointIndex, CircularBuffer<TSample> const &iBuffer) const;

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
  void setZoomFactor(double iZoomFactorPercent);

  /**
   * change the zoom factor while making sure it zooms "around" iInputPageOffset
   * @return the new iInputHistoryOffset
   */
  //int setZoomFactor(double iZoomFactorPercent, int iInputPageOffset, CircularBuffer<TSample> const &iBuffer);

//  inline bool nextZoomedValue(TSample iSample, TSample &oNextZoomedValue)
//  {
//    return fZoom.nextZoomedValue(iSample, oNextZoomedValue);
//  };

//  TSample computeZoomValue(int iInputPageOffset, CircularBuffer<TSample> const &iBuffer) const;
//
  void computeZoomWindow(CircularBuffer<TSample> const &iBuffer, TSample *samples);

//  void setWindowOffset(int iInputHistoryOffset);

private:
  // here iIdx is relative to the right of the screen (see fWindowOffset)
  Zoom<ZOOM_BATCH_SIZE>::ZoomedBuffer zoomFromIndex(int iIdx, CircularBuffer<TSample> const &iBuffer) const;


  // Convenient method to compute the zoom point at the left of the LCD screen
  Zoom<ZOOM_BATCH_SIZE>::ZoomedBuffer zoomFromLeftOfScreen(CircularBuffer<TSample> const &iBuffer) const;

//
//  /**
//   * Computes the zoom value from a zoom point
//   */
//  TSample computeZoomValue(ZoomedBuffer const &iZoomPoint, CircularBuffer<TSample> const &iBuffer) const;

  /**
   * Find the zoom point so that zp.fBufferOffset is the closest to iBufferOffset and returns
   * the index. Also tries to find the point where the zoom value would be the closest.
   */
  //int findClosestWindowIndex(int iBufferOffset, TSample const &iZoomValue, CircularBuffer<TSample> const &iBuffer) const;

  inline int minWindowIdx() const
  {
    return fMinWindowOffset - fVisibleWindowSize + 1;
  }

private:
  int const fVisibleWindowSize;
  int const fBufferSize;

//  // Since points repeat, there is no need to keep more (max repeat = 10 points!)
//  ZoomedBuffer fPoints[MAX_ZOOM_POINTS];
//
//  // how many samples does fPoints represents
//  int fPointsSizeInSamples;

  // offset in the zoomed window (with -1 being the right of the LCD screen for most recent point)
  // this assumes that it corresponds to -1 in fBuffer as well
  int fWindowOffset;

  // the minimum window offset to fit inside the buffer
  int fMinWindowOffset;

  // the maximum amount of zoom allowed
  int fMaxZoomFactor;

  /**
   * Zoom associated to this window */
  Zoom<ZOOM_BATCH_SIZE> fZoom;
};

////////////////////////////////////////////////////////////
// ZoomedBuffer::ZoomedBuffer
////////////////////////////////////////////////////////////
template<int batchSize>
Zoom<batchSize>::ZoomedBuffer::ZoomedBuffer(Zoom<batchSize> const &iZoom,
                                            CircularBuffer<TSample> const &iBuffer, int iBufferOffset,
                                            int iBatchSizeIdx) :
  fZoom{iZoom},
  fBuffer{iBuffer},
  fBufferOffset{iBufferOffset},
  fAccumulatedMax{0},
  fAccumulatedSamples{0},
  fBatchSizeIdx{iBatchSizeIdx}
{
  // nothing to do
}

////////////////////////////////////////////////////////////
// ZoomedBuffer::nextSample
////////////////////////////////////////////////////////////
template<int batchSize>
TSample Zoom<batchSize>::ZoomedBuffer::nextSample()
{
  int numSamples = fZoom.fBatchSizes[fBatchSizeIdx];

  TSample max = 0;
  for(int i = 0; i < numSamples; i++)
  {
    max = std::max(max, fBuffer.getAt(fBufferOffset++));
  }

  fBatchSizeIdx++;
  if(fBatchSizeIdx == batchSize)
    fBatchSizeIdx = 0;

  return max;
}

////////////////////////////////////////////////////////////
// Zoom::zoomFromIndex
////////////////////////////////////////////////////////////
template<int batchSize>
typename Zoom<batchSize>::ZoomedBuffer Zoom<batchSize>::zoomFromIndex(int iZoomPointIndex, CircularBuffer<TSample> const &iBuffer) const
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

  return Zoom<batchSize>::ZoomedBuffer(*this, iBuffer, offset + fOffset[batchSizeIndex], batchSizeIndex);
}

////////////////////////////////////////////////////////////
// Zoom::setZoomFactor
////////////////////////////////////////////////////////////
template<int batchSize>
void Zoom<batchSize>::setZoomFactor(double iZoomFactor)
{
  DCHECK_F(iZoomFactor >= 1.0);

  fZoomFactor = static_cast<int>(iZoomFactor * batchSize);
  init();
}

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

}
}
}