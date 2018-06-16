#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <algorithm>
#include "CircularBuffer.h"

namespace pongasoft {
namespace VST {
namespace Common {

const int FLOAT_TO_INT_FACTOR = 10;

const int MAX_ZOOM_POINTS = 10; // computed in zoom.groovy

using TSample = Steinberg::Vst::Sample64;

class ZoomAlgorithm
{
public:
  inline void start(TSample iSample)
  {
    fWindowPoint = iSample;
  }

  inline void next(TSample iSample)
  {
    fWindowPoint = std::max(fWindowPoint, iSample);
  }

  inline TSample end(TSample iSample)
  {
    fWindowPoint = std::max(fWindowPoint, iSample);

    return fWindowPoint;
  }

private:
  TSample fWindowPoint{0};
};

/**
 * Maintains the current state of zoom */
class Zoom
{
public:
  Zoom();

  explicit Zoom(double iZoomFactor);

  /**
   * @param iZoomFactor zoom factor is 1.0 for min zoom. 2.0 for example means twice as big... etc...
   */
  void setZoomFactor(double iZoomFactor);

  inline bool isNoZoom() const
  { return fZoomFactor == FLOAT_TO_INT_FACTOR; }

  bool nextZoomedValue(TSample iSample, TSample &oNextZoomedValue);

  void reset();

private:
  friend class ZoomWindow;

  void reset(TSample iSample, int iPercent);

  int fZoomFactor;
  ZoomAlgorithm fZoomAlgorithm;
  int fLastPercent;
};

/**
 * Represents a zoom window
 */
class ZoomWindow
{
public:
  class IZoomCallback
  {
  public:
    virtual bool zoomValue(int iIdx, TSample const &iValue) = 0;
  };

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
  int setZoomFactor(double iZoomFactorPercent, int iInputPageOffset, CircularBuffer<TSample> const &iBuffer);

  inline bool nextZoomedValue(TSample iSample, TSample &oNextZoomedValue)
  {
    return fZoom.nextZoomedValue(iSample, oNextZoomedValue);
  };

  TSample computeZoomValue(int iInputPageOffset, CircularBuffer<TSample> const &iBuffer) const;

  bool computeZoomWindow(CircularBuffer<TSample> const &iBuffer, IZoomCallback &callback);

  void computeZoomWindow(CircularBuffer<TSample> const &iBuffer, TSample *samples);

  void setWindowOffset(int iInputHistoryOffset);

private:
  /**
   * A single point in the ZoomWindow
   */
  struct ZoomPoint
  {
    inline ZoomPoint() : fPercentFirstSample(FLOAT_TO_INT_FACTOR), fBufferOffset(-1)
    {}

    // the percentage of the first sample (using FLOAT_TO_INT_FACTOR as 100%)
    int fPercentFirstSample;

    // the offset where this zoom point starts in the underlying buffer (fBuffer)
    int fBufferOffset;
  };

private:
  // here iIdx is relative to the right of the screen (see fWindowOffset)
  ZoomPoint getZoomPoint(int iIdx) const;

  // Convenient method to compute the zoom point at the left of the LCD screen
  ZoomPoint getZoomPointLeftOfScreen() const;

  /**
   * Computes the zoom value from a zoom point
   */
  TSample computeZoomValue(ZoomPoint const &iZoomPoint, CircularBuffer<TSample> const &iBuffer) const;

  /**
   * Find the zoom point so that zp.fBufferOffset is the closest to iBufferOffset and returns
   * the index. Also tries to find the point where the zoom value would be the closest.
   */
  int findClosestWindowIndex(int iBufferOffset, TSample const &iZoomValue, CircularBuffer<TSample> const &iBuffer) const;

  inline int minWindowIdx() const
  { return fMinWindowOffset - fVisibleWindowSize + 1; }

private:
  int const fVisibleWindowSize;
  int const fBufferSize;

  // Since points repeat, there is no need to keep more (max repeat = 10 points!)
  ZoomPoint fPoints[MAX_ZOOM_POINTS];

  // how many samples does fPoints represents
  int fPointsSizeInSamples;

  // offset in the zoomed window (with -1 being the right of the LCD screen for most recent point)
  // this assumes that it corresponds to -1 in fBuffer as well
  int fWindowOffset;

  // the minimum window offset to fit inside the buffer
  int fMinWindowOffset;

  // the maximum amount of zoom allowed
  int fMaxZoomFactor;

  /**
   * Zoom associated to this window */
  Zoom fZoom;
};

}
}
}