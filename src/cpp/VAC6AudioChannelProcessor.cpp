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
VAC6AudioChannelProcessor::VAC6AudioChannelProcessor(SampleRateBasedClock const &iClock) :
  fClock{iClock},
  fMaxAccumulatorForBuffer(fClock.getSampleCountFor(ACCUMULATOR_BATCH_SIZE_IN_MS)),
  fMaxBuffer{new CircularBuffer<TSample>(
    static_cast<int>(ceil(fClock.getSampleCountFor(HISTORY_SIZE_IN_SECONDS * 1000) / fMaxAccumulatorForBuffer.getBatchSize())))},
  fMaxLevelAccumulator(fClock.getSampleCountFor(DEFAULT_MAX_LEVEL_RESET_IN_SECONDS * 1000)),
  fMaxLevel{-1},
  fZoomWindow{MAX_ARRAY_SIZE, fMaxBuffer->getSize()},
  fZoomMaxAccumulator{fZoomWindow.setZoomFactor(DEFAULT_ZOOM_FACTOR_X)},
  fZoomMaxBuffer{new CircularBuffer<TSample>(fZoomWindow.getVisibleWindowSizeInPoints())},
  fNeedToRecomputeZoomMaxBuffer{false},
  fIsLiveView{true},
  fPausedZoomMaxBufferOffset{-1}
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
  if(fIsLiveView == iIsLiveView)
    return;

  fIsLiveView = iIsLiveView;

  resetMaxLevelAccumulator();

  if(!iIsLiveView)
  {
    fMaxLevel = fZoomMaxBuffer->getAt(fPausedZoomMaxBufferOffset);
  }
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::setLCDInputX
/////////////////////////////////////////
void VAC6AudioChannelProcessor::setLCDInputX(int iLCDInputX)
{
  // iLCDInputX [0-255] => offset [-256, -1]
  setPausedZoomMaxBufferOffset(iLCDInputX - MAX_ARRAY_SIZE);
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::setPausedZoomMaxBufferOffset
/////////////////////////////////////////
void VAC6AudioChannelProcessor::setPausedZoomMaxBufferOffset(int iOffset)
{
  fPausedZoomMaxBufferOffset = iOffset;

  if(!fIsLiveView)
  {
    fMaxLevel = fZoomMaxBuffer->getAt(fPausedZoomMaxBufferOffset);
  }
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::setHistoryOffset
/////////////////////////////////////////
void VAC6AudioChannelProcessor::setHistoryOffset(double iHistoryOffset)
{
  fZoomWindow.setWindowOffset(iHistoryOffset);
  fNeedToRecomputeZoomMaxBuffer = true;
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::setZoomFactor
/////////////////////////////////////////
void VAC6AudioChannelProcessor::setZoomFactor(double iZoomFactorPercent)
{
  fZoomWindow.setZoomFactor(iZoomFactorPercent);
  fNeedToRecomputeZoomMaxBuffer = true;
}

/////////////////////////////////////////
// VAC6AudioChannelProcessor::setZoomFactor
/////////////////////////////////////////
void VAC6AudioChannelProcessor::setZoomFactor(double iZoomFactorPercent, int iLCDInputX)
{
  fZoomWindow.setZoomFactor(iZoomFactorPercent, iLCDInputX);
  fNeedToRecomputeZoomMaxBuffer = true;
}


/////////////////////////////////////////
// VAC6AudioChannelProcessor::genericProcessChannel
/////////////////////////////////////////
// => implemented in VAC6Processor.cpp (because it is generic)

}
}
}
