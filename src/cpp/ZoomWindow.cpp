
#include "ZoomWindow.h"
#include "AudioUtils.h"

namespace pongasoft {
namespace VST {
namespace Common {

const int MIN_HISTORY_OFFSET = 0;
const int MAX_HISTORY_OFFSET = 10000;
int const MAX_WINDOW_OFFSET = -1;

////////////////////////////////////////////////////////////
// ZoomWindow
////////////////////////////////////////////////////////////

ZoomWindow::ZoomWindow(int iVisibleWindowSize, int iBufferSize) :
  fVisibleWindowSize(iVisibleWindowSize),
  fWindowOffset(MAX_WINDOW_OFFSET),
  fBufferSize(iBufferSize)
{
  DCHECK_GT_F(iVisibleWindowSize, 0);

  fMaxZoomFactor = fBufferSize / iVisibleWindowSize;

  setZoomFactor(1.0);
}

////////////////////////////////////////////////////////////
// ZoomWindow::setZoomFactor
// Recomputes the zoom points based on the factor
////////////////////////////////////////////////////////////
TZoom::MaxAccumulator ZoomWindow::setZoomFactor(double iZoomFactorPercent)
{
  auto accumulator = fZoom.setZoomFactor(iZoomFactorPercent * (1 - fMaxZoomFactor) + fMaxZoomFactor);

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
  assert(fMinWindowOffset <= MAX_WINDOW_OFFSET);

  // we make sure that fWindowOffset remains within its bounds!
  fWindowOffset = clamp(fWindowOffset, fMinWindowOffset, MAX_WINDOW_OFFSET);

  return accumulator;
}

////////////////////////////////////////////////////////////
// ZoomWindow::setZoomFactor
// Recomputes the zoom points based on the factor and page offset
////////////////////////////////////////////////////////////
//int ZoomWindow::setZoomFactor(double iZoomFactorPercent, int iInputPageOffset, CircularBuffer<TSample> const &iBuffer)
//{
//  // index in the window
//  int windowIdx = fWindowOffset - fVisibleWindowSize + 1 + iInputPageOffset;
//
//  // first determine the offset prior to zooming from the window idx
//  int bufferOffset;
//  TSample zoomValue;
//
//  if(fZoom.isNoZoom())
//    bufferOffset = windowIdx;
//  else
//  {
//    const ZoomedBuffer &zoomPoint = zoom(windowIdx);
//    bufferOffset = zoomPoint.fBufferOffset;
//    zoomValue = computeZoomValue(zoomPoint, iBuffer);
//  }
//
//  // second apply zoom => this will change fPoints, fMinWindowOffset, etc...
//  setZoomFactor(iZoomFactorPercent);
//
//  // when we are all the way on the right no need to compute anything
//  if(windowIdx == MAX_WINDOW_OFFSET)
//    return MAX_HISTORY_OFFSET;
//
//  // when we are all the way on the left no need to compute anything
//  if(windowIdx == fMinWindowOffset)
//    return MIN_HISTORY_OFFSET;
//
//  // third determine new window offset
//  if(fZoom.isNoZoom())
//    windowIdx = bufferOffset;
//  else
//    windowIdx = findClosestWindowIndex(bufferOffset, zoomValue, iBuffer);
//
//  // we compute the window offset from the window index
//  fWindowOffset = windowIdx - iInputPageOffset + fVisibleWindowSize - 1;
//
//  // we make sure that fWindowOffset remains within its bounds!
//  fWindowOffset = clamp(fWindowOffset, fMinWindowOffset, MAX_WINDOW_OFFSET);
//
//  if(fMinWindowOffset == MAX_WINDOW_OFFSET)
//    return 0;
//
//  // finally compute new inputHistoryOffset
//  auto inputHistoryOffset =
//    static_cast<int>(round((fWindowOffset - fMinWindowOffset) * static_cast<float>(MAX_HISTORY_OFFSET) /
//                           (MAX_WINDOW_OFFSET - fMinWindowOffset)));
//
//  return inputHistoryOffset;
//}

////////////////////////////////////////////////////////////
// ZoomWindow::computeZoomWindow
////////////////////////////////////////////////////////////
TZoom::MaxAccumulator ZoomWindow::computeZoomWindow(CircularBuffer<TSample> const &iBuffer, CircularBuffer<TSample> &oBuffer) const
{
  DCHECK_EQ_F(fBufferSize, iBuffer.getSize());
  DCHECK_EQ_F(fVisibleWindowSize, oBuffer.getSize());

  int offset = 0;
  auto accumulator = getMaxAccumulatorFromLeftOfScreen(offset);

  TSample max;
  for(; offset < 0; offset++)
  {
    if(accumulator.accumulate(iBuffer.getAt(offset), max))
      oBuffer.push(max);
  }

  return accumulator;
}

////////////////////////////////////////////////////////////
// ZoomWindow::getMaxAccumulatorFromIndex
////////////////////////////////////////////////////////////
TZoom::MaxAccumulator ZoomWindow::getMaxAccumulatorFromIndex(int iIdx, int &oOffset) const
{
  DCHECK_F(iIdx >= minWindowIdx() && iIdx <= MAX_WINDOW_OFFSET);

  auto accumulator = fZoom.getAccumulatorFromIndex(iIdx, oOffset);
  DCHECK_F(oOffset >= -fBufferSize);
  return accumulator;
}

////////////////////////////////////////////////////////////
// ZoomWindow::getMaxAccumulatorFromLeftOfScreen
////////////////////////////////////////////////////////////
TZoom::MaxAccumulator ZoomWindow::getMaxAccumulatorFromLeftOfScreen(int &oOffset) const
{
  return getMaxAccumulatorFromIndex(fWindowOffset - fVisibleWindowSize + 1, oOffset);
}

////////////////////////////////////////////////////////////
// ZoomWindow::findClosestWindowIndex
////////////////////////////////////////////////////////////
//int ZoomWindow::findClosestWindowIndex(int iBufferOffset, TSample const &iZoomValue, CircularBuffer<TSample> const &iBuffer) const
//{
//  DCHECK_EQ_F(fBufferSize, iBuffer.getSize());
//
//  int i1 = minWindowIdx();
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
//    int idx = clamp(mid + i, minWindowIdx(), MAX_WINDOW_OFFSET);
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
//void ZoomWindow::setWindowOffset(int iInputHistoryOffset)
//{
//  switch(iInputHistoryOffset)
//  {
//    case MIN_HISTORY_OFFSET:
//      fWindowOffset = fMinWindowOffset;
//      break;
//
//    case MAX_HISTORY_OFFSET:
//      fWindowOffset = MAX_WINDOW_OFFSET;
//      break;
//
//    default:
//      fWindowOffset = (int)
//        (static_cast<float>(MAX_WINDOW_OFFSET - fMinWindowOffset) / MAX_HISTORY_OFFSET * iInputHistoryOffset +
//         fMinWindowOffset);
//      break;
//  }
//
//  // sanity check
//  assert(fWindowOffset >= fMinWindowOffset && fWindowOffset <= MAX_WINDOW_OFFSET);
//}

}
}
}