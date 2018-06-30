#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <algorithm>
#include <cmath>
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

  /**
   * Class which helps in accumulating samples when zoomed. Zoom::fBatchSizes is used to keep track of how many
   * samples need to be accumulated to produce the next one.
   */
  class MaxAccumulator
  {
  public:
    // Constructor
    explicit MaxAccumulator(class_type const *iZoom) :
      fZoom{iZoom},
      fAccumulatedMax{0},
      fAccumulatedSamples{0},
      fBatchSizeIdx{0}
    {
    }

    // Constructor when not starting from 0
    MaxAccumulator(class_type const *iZoom, int iBatchSizeIdx) :
      fZoom{iZoom},
      fAccumulatedMax{0},
      fAccumulatedSamples{0},
      fBatchSizeIdx{iBatchSizeIdx}
    {
    }

    /**
     * Accumulate the given sample. If a batch is complete (as defined by the Zoom) then it returns true and
     * populate oMaxSample with the value. Otherwise it returns false and does not modify oMaxSample
     */
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

    // getAccumulatedMax
    TSample getAccumulatedMax() const
    {
      return fAccumulatedMax;
    }

    // getBatchSizeIdx
    int getBatchSizeIdx() const
    {
      return fBatchSizeIdx;
    }

    // getAccumulatedSamples
    int getAccumulatedSamples() const
    {
      return fAccumulatedSamples;
    }

  private:
    class_type const *fZoom;

    TSample fAccumulatedMax;
    int fAccumulatedSamples;
    int fBatchSizeIdx;
  };

public:
  friend class class_type::MaxAccumulator;

  // Constructor
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
   * This is the reverse of the previous getAccumulatorFromIndex.
   *
   * @param iOffset the offset on where the start in the non zoomed buffer
   * @return the equivalent zoom point
   */
  int getZoomPointIndexFromOffset(int iOffset) const;

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

  inline int getIndexFromOffset(int iOffset) const
  {
    DCHECK_F(iOffset >= 0 && iOffset < getBatchSizeInSamples());

    for(int i = 0; i < batchSize; i++)
    {
      if(iOffset < fOffset[i])
        return i;
    }

    // never reached!
    DCHECK_F(false);

    return batchSize - 1;
  }

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
   * Updates the zoom factor by using the iOffsetFromLeftOfScreen as the reference point */
  void setZoomFactor(double iZoomFactorPercent, int iOffsetFromLeftOfScreen);

  /**
   * Changes the window offset. Note that window offset is an "abstract" value given as a percentage so that it does
   * not depend on the level of zoom. The internal window offset (fWindowOffset) is adjusted to the actual position
   * in the buffer accounting zoom level.
   */
  void setWindowOffset(double iWindowOffsetPercent);

  /**
   * Return the visible size of the window (number of points) */
  inline int getVisibleWindowSizeInPoints() const
  {
    return fVisibleWindowSize;
  }

  /**
   * @return the number of samples displayed in the window (when not zoomed,
   * getVisibleWindowSizeInSamples() == getVisibleWindowSizeInPoints(), if zoom is 2x,
   * then getVisibleWindowSizeInSamples() == 2 * getVisibleWindowSizeInPoints() etc...)
   */
  inline int getVisibleWindowSizeInSamples() const
  {
    return static_cast<int>(ceil(getVisibleWindowSizeInPoints() * fZoom.getBatchSizeInSamples() / static_cast<double>(fZoom.getBatchSize())));
  }

  /**
   * Computes the zoom
   * @param iBuffer
   * @param oBuffer
   */
  TZoom::MaxAccumulator computeZoomWindow(const CircularBuffer <TSample> &iBuffer, CircularBuffer <TSample> &oBuffer) const;

  /////////////////////////////////////////////////////////////////////
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // !! The methods below are public only for the purpose of testing !!
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  /////////////////////////////////////////////////////////////////////
public:
  // here iIdx is relative to the right of the screen (see fWindowOffset)
  TZoom::MaxAccumulator __getMaxAccumulatorFromIndex(int iIdx, int &oOffset) const;


  // Convenient method to compute the zoom point at the left of the LCD screen
  TZoom::MaxAccumulator __getMaxAccumulatorFromLeftOfScreen(int &oOffset) const;

  /**
   * @param iZoomFactor the zoom factor with 1.0 being no zoom, 2.0 being 2x, etc... (used internally)
   */
  TZoom::MaxAccumulator __setRawZoomFactor(double iZoomFactor);

  /**
   * Changes the window offset to the given value (used internally)
   */
  void __setRawWindowOffset(int iWindowOffset);

  inline int __getWindowOffset() const
  {
    return fWindowOffset;
  }

  // __getMinWindowIdx
  inline int __getMinWindowIdx() const
  {
    return fMinWindowOffset - fVisibleWindowSize + 1;
  }

  // __getMinWindowOffset
  inline int __getMinWindowOffset() const
  {
    return fMinWindowOffset;
  }

  // __getMaxZoomFactor
  inline double __getMaxZoomFactor() const
  {
    return fMaxZoomFactor;
  }

private:
  int const fVisibleWindowSize;
  int const fBufferSize;

  // offset in the zoomed window (with -1 being the right of the LCD screen for most recent point)
  // this is a negative index in the zoomed buffer
  int fWindowOffset;

  // the minimum window offset to fit inside the buffer
  int fMinWindowOffset;

  // the maximum amount of zoom allowed
  double fMaxZoomFactor;

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
    totalBatchSize += accumulatedSamples;
    accumulatedSamples = 0;

    accumulatedZoom = newAccumulatedZoom;
  }

  DCHECK_EQ_F(totalBatchSize, fZoomFactor);

  int accumulatedOffset = 0;
  for(int i = batchSize - 1; i >= 0; i--)
  {
    accumulatedOffset -= fBatchSizes[i];
    fOffset[i] = accumulatedOffset;
  }
}

////////////////////////////////////////////////////////////
// Zoom::getAccumulatorFromIndex
////////////////////////////////////////////////////////////
template<int batchSize>
typename Zoom<batchSize>::MaxAccumulator Zoom<batchSize>::getAccumulatorFromIndex(int iZoomPointIndex, int &oOffset) const
{
  // the first element in the offset is -1 so it should always be a negative number
  DCHECK_F(iZoomPointIndex < 0);

  // this gives un a number between ]-batchSize, 0]
  int batchSizeIndex = iZoomPointIndex % batchSize;

  // we bring it back to [0, batchSize[
  if(batchSizeIndex != 0)
    batchSizeIndex += batchSize;

  DCHECK_F(batchSizeIndex >= 0 && batchSizeIndex < batchSize);

  int offset = ((iZoomPointIndex + 1) / batchSize) * getBatchSizeInSamples(); // multiple of getBatchSizeInSamples
  oOffset = offset + fOffset[batchSizeIndex];

  return Zoom<batchSize>::MaxAccumulator(this, batchSizeIndex);
}

////////////////////////////////////////////////////////////
// Zoom::getAccumulatorFromOffset
////////////////////////////////////////////////////////////
template<int batchSize>
int Zoom<batchSize>::getZoomPointIndexFromOffset(int iOffset) const
{
  // iOffset should be negative
  DCHECK_F(iOffset < 0);

  int offset = iOffset % getBatchSizeInSamples();

  // make sure it is in the [0, getBatchSizeInSamples()[ range
  if(offset != 0)
    offset += getBatchSizeInSamples();

  auto zpi = (iOffset / getBatchSizeInSamples()) * batchSize;

  zpi += getIndexFromOffset(offset);

  return zpi;
}

}
}
}