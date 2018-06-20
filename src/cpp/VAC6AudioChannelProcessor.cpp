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
  fZoomMaxBuffer{new CircularBuffer<TSample>(fZoomWindow.getVisibleWindowSizeInPoints())}
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
// VAC6AudioChannelProcessor::genericProcessChannel
/////////////////////////////////////////
// => implemented in VAC6Processor.cpp (because it is generic)

}
}
}
