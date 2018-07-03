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
  fMaxLevelSinceLastReset{0},
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
// VAC6AudioChannelProcessor::setMaxLevelMode
/////////////////////////////////////////
void VAC6AudioChannelProcessor::setMaxLevelMode(MaxLevelMode iMaxLevelMode)
{
  fMaxLevelMode = iMaxLevelMode;
  fMaxLevelSinceLastReset = 0;
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::computeMaxLevelInSinceResetMode
/////////////////////////////////////////
MaxLevel VAC6AudioChannelProcessor::computeMaxLevelInSinceResetMode() const
{
  MaxLevel maxLevel{fMaxLevelSinceLastReset, -1};

  // the purpose is to find the highest index where level == fMaxLevelSinceLastReset (the window may not contain it
  // in which case it will remain -1)
  auto findIndex = [&maxLevel] (int index, int const &iPreviousIndex, double const &iLevel) -> int {
    if(iLevel == maxLevel.fValue)
      return index;
    else
      return iPreviousIndex;
  };

  maxLevel.fIndex = fZoomMaxBuffer->foldWithIndex<int>(-1, findIndex);

  return maxLevel;
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::computeMaxLevelInWindowMode
/////////////////////////////////////////
MaxLevel VAC6AudioChannelProcessor::computeMaxLevelInWindowMode() const
{
  MaxLevel maxLevel{};

  // the purpose is to find the max level in the window and if there is more than one return the highest level
  auto findIndex = [&maxLevel] (int index, int const &iPreviousIndex, double const &iLevel) -> int {
    if(iLevel >= maxLevel.fValue)
    {
      maxLevel.fValue = iLevel;
      return index;
    }
    else
      return iPreviousIndex;
  };

  maxLevel.fIndex = fZoomMaxBuffer->foldWithIndex<int>(-1, findIndex);

  return maxLevel;
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::genericProcessChannel
/////////////////////////////////////////
// => implemented in VAC6Processor.cpp (because it is generic)

}
}
}
