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
                                                     int iMaxBufferSize) :
  fClock{iClock},
  fMaxAccumulatorForBuffer(fClock.getSampleCountFor(ACCUMULATOR_BATCH_SIZE_IN_MS)),
  fMaxBuffer{new CircularBuffer<TSample>(iMaxBufferSize)},
  fMaxLevelSinceReset{0},
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
// VAC6AudioChannelProcessor::genericProcessChannel
/////////////////////////////////////////
// => implemented in VAC6Processor.cpp (because it is generic)

}
}
}
