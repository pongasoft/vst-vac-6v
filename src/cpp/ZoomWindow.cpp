
#include "ZoomWindow.h"
#include "AudioUtils.h"
#include "Utils.h"

namespace pongasoft {
namespace VST {
namespace Common {

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
  return __setRawZoomFactor(getZoomFactorLerp().compute(iZoomFactorPercent));
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

  // zooming always happen at the right of the screen
  fWindowOffset = MAX_WINDOW_OFFSET;

  return accumulator;
}

////////////////////////////////////////////////////////////
// ZoomWindow::setZoomFactor
////////////////////////////////////////////////////////////
int ZoomWindow::setZoomFactor(double iZoomFactorPercent, int iOffsetFromLeftOfScreen, std::initializer_list<CircularBuffer<TSample> const *>iBuffers)
{
  DCHECK_F(iZoomFactorPercent >= 0 && iZoomFactorPercent <= 1.0);
  return __setRawZoomFactor(getZoomFactorLerp().compute(iZoomFactorPercent), iOffsetFromLeftOfScreen, iBuffers);
}

////////////////////////////////////////////////////////////
// ZoomWindow::__setRawZoomFactor
////////////////////////////////////////////////////////////
int ZoomWindow::__setRawZoomFactor(double iZoomFactor,
                                   int iOffsetFromLeftOfScreen,
                                   std::initializer_list<CircularBuffer<TSample> const *>iBuffers)
{
  DCHECK_F(iZoomFactor >= 1.0 && iZoomFactor <= fMaxZoomFactor);
  DCHECK_F(iOffsetFromLeftOfScreen >= 0 && iOffsetFromLeftOfScreen < fVisibleWindowSize);

  // first we compute the zoom point index (in the entire zoomed history) prior to zooming
  int beforeZoomPointIndex = fWindowOffset - fVisibleWindowSize + 1 + iOffsetFromLeftOfScreen;

  // we capture the offset prior to zooming
  int maxOffset = 0;
  TSample max = 0;

  for(auto buffer : iBuffers)
  {
    if(buffer)
    {
      int newMaxOffset = 0;
      TSample newMax = __findMaxForIndex(beforeZoomPointIndex, *buffer, newMaxOffset);
      if(newMax > max)
      {
        max = newMax;
        maxOffset = newMaxOffset;
      }
    }
  }

  // when we don't have any buffers to find the max, simply approximate it...
  if(maxOffset >= 0)
  {
    __getMaxAccumulatorFromIndex(beforeZoomPointIndex, maxOffset);
  }

  // we now apply the zoom
  __setRawZoomFactor(iZoomFactor);

  // now we determine the zoom point index (in the entire zoomed history) after zooming
  int afterZoomPointIndex = fZoom.getZoomPointIndexFromOffset(maxOffset);

  // we try to maintain the iOffsetFromLeftOfScreen at the same location (which is not always possible)
  int newWindowOffset = afterZoomPointIndex + fVisibleWindowSize - 1 - iOffsetFromLeftOfScreen;

  if(newWindowOffset < fMinWindowOffset)
  {
    iOffsetFromLeftOfScreen -= fMinWindowOffset - newWindowOffset;
    fWindowOffset = fMinWindowOffset;
  }
  else
  {
    if(newWindowOffset > MAX_WINDOW_OFFSET)
    {
      iOffsetFromLeftOfScreen += newWindowOffset - MAX_WINDOW_OFFSET;
      fWindowOffset = MAX_WINDOW_OFFSET;
    }
    else
    {
      fWindowOffset = newWindowOffset;
    }
  }

  iOffsetFromLeftOfScreen = clamp(iOffsetFromLeftOfScreen, 0, fVisibleWindowSize - 1);

  return iOffsetFromLeftOfScreen;
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
  DCHECK_F(oOffset >= -fBufferSize && oOffset <= -1);
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
// ZoomWindow::__getMaxAccumulatorFromIndex
////////////////////////////////////////////////////////////
TSample ZoomWindow::__findMaxForIndex(int iIdx, CircularBuffer<TSample> const &iBuffer, int &oMaxOffset) const
{
  DCHECK_F(iIdx >= __getMinWindowIdx() && iIdx <= MAX_WINDOW_OFFSET);

  int firstOffsetInBatch;
  auto accumulator = __getMaxAccumulatorFromIndex(iIdx, firstOffsetInBatch);

  int nextOffset = 0;

  return accumulator.accumulate<TSample>(iBuffer, firstOffsetInBatch, oMaxOffset, nextOffset);
}

////////////////////////////////////////////////////////////
// ZoomWindow::setWindowOffset
////////////////////////////////////////////////////////////
void ZoomWindow::setWindowOffset(double iWindowOffsetPercent)
{
  DCHECK_F(iWindowOffsetPercent >= 0 && iWindowOffsetPercent <= 1.0);

  auto lerp = getWindowOffsetLerp();
  __setRawWindowOffset(static_cast<int>(lerp.compute(iWindowOffsetPercent)));
}

////////////////////////////////////////////////////////////
// ZoomWindow::setWindowOffset
////////////////////////////////////////////////////////////
double ZoomWindow::getWindowOffset() const
{
  if(fMinWindowOffset == MAX_WINDOW_OFFSET)
    return 1.0;

  auto lerp = getWindowOffsetLerp();
  return lerp.reverse(fWindowOffset);
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