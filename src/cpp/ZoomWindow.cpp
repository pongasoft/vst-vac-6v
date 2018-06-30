
#include "ZoomWindow.h"
#include "AudioUtils.h"
#include "Utils.h"

namespace pongasoft {
namespace VST {
namespace Common {

constexpr int MAX_WINDOW_OFFSET = -1;

////////////////////////////////////////////////////////////
// ZoomWindow
////////////////////////////////////////////////////////////

ZoomWindow::ZoomWindow(int iVisibleWindowSize, int iBufferSize) :
  fVisibleWindowSize(iVisibleWindowSize),
  fWindowOffset(MAX_WINDOW_OFFSET),
  fBufferSize(iBufferSize)
{
  DCHECK_GT_F(iVisibleWindowSize, 0);

  fMaxZoomFactor = static_cast<double>(fBufferSize) / iVisibleWindowSize;

  setZoomFactor(1.0);
}

////////////////////////////////////////////////////////////
// ZoomWindow::setZoomFactor
////////////////////////////////////////////////////////////
TZoom::MaxAccumulator ZoomWindow::setZoomFactor(double iZoomFactorPercent)
{
  DCHECK_F(iZoomFactorPercent >= 0 && iZoomFactorPercent <= 1.0);
  return __setRawZoomFactor(Utils::Lerp<double>(fMaxZoomFactor, 1.0).compute(iZoomFactorPercent));
}

////////////////////////////////////////////////////////////
// ZoomWindow::__setRawZoomFactor
////////////////////////////////////////////////////////////
TZoom::MaxAccumulator ZoomWindow::__setRawZoomFactor(double iZoomFactor)
{
  DCHECK_F(iZoomFactor >= 1.0 && iZoomFactor <= fMaxZoomFactor);

  auto accumulator = fZoom.setZoomFactor(iZoomFactor);

  if(fZoom.isNoZoom())
  {
    fMinWindowOffset = -fBufferSize + fVisibleWindowSize - 1;
  }
  else
  {
    fMinWindowOffset =
      -(int) (((float) fBufferSize) / fZoom.getBatchSizeInSamples() * fZoom.getBatchSize()) + fVisibleWindowSize - 1;
  }

  // should never happen...
  DCHECK_F(fMinWindowOffset <= MAX_WINDOW_OFFSET);

  // we make sure that fWindowOffset remains within its bounds!
  fWindowOffset = clamp(fWindowOffset, fMinWindowOffset, MAX_WINDOW_OFFSET);

  return accumulator;
}

////////////////////////////////////////////////////////////
// ZoomWindow::setZoomFactor
////////////////////////////////////////////////////////////
void ZoomWindow::setZoomFactor(double iZoomFactorPercent, int iOffsetFromLeftOfScreen)
{
  DCHECK_F(iOffsetFromLeftOfScreen >= 0 && iOffsetFromLeftOfScreen < fVisibleWindowSize);

  // index in the window
  int windowIdx = fWindowOffset - fVisibleWindowSize + 1 + iOffsetFromLeftOfScreen;

  // we capture the offset prior to zooming
  int offset = 0;
  __getMaxAccumulatorFromIndex(windowIdx, offset);

  // we now apply the zoom
  setZoomFactor(iZoomFactorPercent);

  int zoomPointIndex = fZoom.getZoomPointIndexFromOffset(offset);

  fWindowOffset = zoomPointIndex - fVisibleWindowSize + 1 + iOffsetFromLeftOfScreen;

  // we make sure that fWindowOffset remains within its bounds!
  fWindowOffset = clamp(fWindowOffset, fMinWindowOffset, MAX_WINDOW_OFFSET);
}

////////////////////////////////////////////////////////////
// ZoomWindow::computeZoomWindow
////////////////////////////////////////////////////////////
TZoom::MaxAccumulator ZoomWindow::computeZoomWindow(CircularBuffer<TSample> const &iBuffer, CircularBuffer<TSample> &oBuffer) const
{
  DCHECK_EQ_F(fBufferSize, iBuffer.getSize());
  DCHECK_EQ_F(fVisibleWindowSize, oBuffer.getSize());

  int offset = 0;
  auto accumulator = __getMaxAccumulatorFromLeftOfScreen(offset);

  TSample max;
  for(int i = 0; i < fVisibleWindowSize; i++)
  {
    while(!accumulator.accumulate(iBuffer.getAt(offset++), max))
    {}
    oBuffer.push(max);
  }

  return accumulator;
}

////////////////////////////////////////////////////////////
// ZoomWindow::__getMaxAccumulatorFromIndex
////////////////////////////////////////////////////////////
TZoom::MaxAccumulator ZoomWindow::__getMaxAccumulatorFromIndex(int iIdx, int &oOffset) const
{
  DCHECK_F(iIdx >= __getMinWindowIdx() && iIdx <= MAX_WINDOW_OFFSET);

  auto accumulator = fZoom.getAccumulatorFromIndex(iIdx, oOffset);
  DCHECK_F(oOffset >= -fBufferSize);
  return accumulator;
}

////////////////////////////////////////////////////////////
// ZoomWindow::__getMaxAccumulatorFromLeftOfScreen
////////////////////////////////////////////////////////////
TZoom::MaxAccumulator ZoomWindow::__getMaxAccumulatorFromLeftOfScreen(int &oOffset) const
{
  return __getMaxAccumulatorFromIndex(fWindowOffset - fVisibleWindowSize + 1, oOffset);
}

////////////////////////////////////////////////////////////
// ZoomWindow::findClosestWindowIndex
////////////////////////////////////////////////////////////
//int ZoomWindow::findClosestWindowIndex(int iBufferOffset, TSample const &iZoomValue, CircularBuffer<TSample> const &iBuffer) const
//{
//  DCHECK_EQ_F(fBufferSize, iBuffer.getSize());
//
//  int i1 = __getMinWindowIdx();
//  int i2 = MAX_WINDOW_OFFSET;
//
//  int mid = MAX_WINDOW_OFFSET;
//
//  ZoomWindow::ZoomedBuffer zp;
//
//  while(i1 <= i2)
//  {
//    mid = i1 + (i2 - i1) / 2;
//
//    zp = zoom(mid);
//
//    if(zp.fBufferOffset == iBufferOffset)
//      break;
//
//    if(zp.fBufferOffset < iBufferOffset)
//      i1 = mid + 1;
//    else
//      i2 = mid - 1;
//  }
//
//  TSample minZoomDelta = fabs(iZoomValue - computeZoomValue(zp, iBuffer));
//  int minBufferOffsetDelta = abs(iBufferOffset - zp.fBufferOffset);
//
//  // we found the exact point... no need to go further...
//  if(minBufferOffsetDelta == 0 && equals5DP(minZoomDelta, 0.0))
//    return mid;
//
//  int res = mid;
//
//  // determine the best point
//  for(int i = -2; i <= 2; i++)
//  {
//    int idx = clamp(mid + i, __getMinWindowIdx(), MAX_WINDOW_OFFSET);
//
//    // skipping mid (already done)
//    if(idx == mid)
//      continue;
//
//    const ZoomedBuffer &point = zoom(idx);
//    TSample delta = fabs(iZoomValue - computeZoomValue(point, iBuffer));
//
//    if(delta < minZoomDelta)
//    {
//      res = idx;
//      minZoomDelta = delta;
//      minBufferOffsetDelta = abs(iBufferOffset - point.fBufferOffset);
//    }
//
//    if(delta == minZoomDelta)
//    {
//      int bufferOffsetDelta = abs(iBufferOffset - point.fBufferOffset);
//      if(bufferOffsetDelta < minBufferOffsetDelta)
//      {
//        res = idx;
//        minZoomDelta = delta;
//        minBufferOffsetDelta = bufferOffsetDelta;
//      }
//    }
//  }
//
//  // did not find an exact match but las one should be "close" enough
//  return res;
//}

////////////////////////////////////////////////////////////
// ZoomWindow::setWindowOffset
////////////////////////////////////////////////////////////
void ZoomWindow::setWindowOffset(double iWindowOffsetPercent)
{
  DCHECK_F(iWindowOffsetPercent >= 0 && iWindowOffsetPercent <= 1.0);

  auto lerp = Utils::Lerp<double>(fMinWindowOffset, MAX_WINDOW_OFFSET);
  __setRawWindowOffset(static_cast<int>(lerp.compute(iWindowOffsetPercent)));
}

////////////////////////////////////////////////////////////
// ZoomWindow::__setRawWindowOffset
////////////////////////////////////////////////////////////
void ZoomWindow::__setRawWindowOffset(int iWindowOffset)
{
  DCHECK_F(iWindowOffset >= fMinWindowOffset && iWindowOffset <= MAX_WINDOW_OFFSET);
  fWindowOffset = iWindowOffset;
}


}
}
}