#include "VAC6AudioChannelProcessor.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg;
using namespace Steinberg::Vst;

using namespace VST::Common;

/////////////////////////////////////////
// VAC6AudioChannelProcessor::VAC6AudioChannelProcessor
/////////////////////////////////////////
VAC6AudioChannelProcessor::VAC6AudioChannelProcessor(const SampleRateBasedClock &iClock,
                                                     ZoomWindow *iZoomWindow,
                                                     uint32 iMaxAccumulatorBatchSize,
                                                     int iMaxBufferSize) :
  fClock{iClock},
  fMaxAccumulatorForBuffer(iMaxAccumulatorBatchSize),
  fMaxBuffer{new CircularBuffer<TSample>(iMaxBufferSize)},
  fMaxLevel{0},
  fMaxLevelIndex{-1},
  fMaxLevelMode{DEFAULT_MAX_LEVEL_MODE},
  fZoomMaxAccumulator{iZoomWindow->setZoomFactor(DEFAULT_ZOOM_FACTOR_X)},
  fZoomMaxBuffer{new CircularBuffer<TSample>(iZoomWindow->getVisibleWindowSizeInPoints())},
  fNeedToRecomputeZoomMaxBuffer{false},
  fIsLiveView{true}
{
  fMaxBuffer->init(0);
  fZoomMaxBuffer->init(0);
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::~VAC6AudioChannelProcessor
/////////////////////////////////////////
VAC6AudioChannelProcessor::~VAC6AudioChannelProcessor()
{
  delete fZoomMaxBuffer;
  delete fMaxBuffer;
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::computeZoomSamples
/////////////////////////////////////////
void VAC6AudioChannelProcessor::computeZoomSamples(int iNumSamples, TSample *oSamples) const
{
  for(int i = 0; i < iNumSamples; i++)
    oSamples[i] = fZoomMaxBuffer->getAt(i);
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::setIsLiveView
/////////////////////////////////////////
void VAC6AudioChannelProcessor::setIsLiveView(bool iIsLiveView)
{
  fIsLiveView = iIsLiveView;
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::setDirty
/////////////////////////////////////////
void VAC6AudioChannelProcessor::setDirty()
{
  fNeedToRecomputeZoomMaxBuffer = true;
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::setMaxLevelIndex
/////////////////////////////////////////
void VAC6AudioChannelProcessor::setMaxLevelIndex(int iMaxLevelIndex)
{
  DCHECK_F(iMaxLevelIndex >= -1 && iMaxLevelIndex < fZoomMaxBuffer->getSize());
  fMaxLevelIndex = iMaxLevelIndex;
  fMaxLevel = fZoomMaxBuffer->getAt(fMaxLevelIndex - fZoomMaxBuffer->getSize());
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::setMaxLevelMode
/////////////////////////////////////////
void VAC6AudioChannelProcessor::setMaxLevelMode(MaxLevelMode iMaxLevelMode)
{
  fMaxLevelMode = iMaxLevelMode;
  adjustMaxLevel();
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::adjustMaxLevel
/////////////////////////////////////////
void VAC6AudioChannelProcessor::adjustMaxLevel()
{
  switch(fMaxLevelMode)
  {
    case kMaxInWindow:
      adjustMaxLevelInWindowMode();
      break;

    case kMaxSinceReset:
      adjustMaxLevelInSinceResetMode();
      break;

    default:
      DCHECK_F(false, "should not be reached");
      break;
  }
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::adjustMaxLevelInSinceResetMode
/////////////////////////////////////////
void VAC6AudioChannelProcessor::adjustMaxLevelInSinceResetMode()
{
  auto maxLevel = fMaxLevel;

  // the purpose is to find the highest index where level == fMaxLevel (the window may not contain it
  // in which case it will remain -1)
  auto findIndex = [&maxLevel] (int index, int const &iPreviousIndex, double const &iLevel) -> int {
    if(iLevel == maxLevel)
      return index;
    else
      return iPreviousIndex;
  };

  int maxLevelIndex = fZoomMaxBuffer->foldWithIndex<int>(-1, findIndex);

  fMaxLevelIndex = maxLevelIndex;

  // DLOG_F(INFO, "adjustISRM %d -> %d", fMaxLevelIndex, maxLevelIndex);
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::adjustMaxLevelInWindowMode
/////////////////////////////////////////
void VAC6AudioChannelProcessor::adjustMaxLevelInWindowMode()
{
  TSample maxLevel = 0.0;

  // the purpose is to find the max level in the window and if there is more than one return the highest level
  auto findIndex = [&maxLevel] (int index, int const &iPreviousIndex, double const &iLevel) -> int {
    if(iLevel >= maxLevel)
    {
      maxLevel = iLevel;
      return index;
    }
    else
      return iPreviousIndex;
  };

  int maxLevelIndex = fZoomMaxBuffer->foldWithIndex<int>(-1, findIndex);

  // DLOG_F(INFO, "adjustIWM %d -> %d, %f -> %f", fMaxLevelIndex, maxLevelIndex, fMaxLevel, maxLevel);

  fMaxLevelIndex = maxLevelIndex;
  fMaxLevel = maxLevel;
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::genericProcessChannel
/////////////////////////////////////////
// => implemented in VAC6Processor.cpp (because it is generic)

}
}
}
