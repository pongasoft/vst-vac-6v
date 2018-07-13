#pragma once

#include <pluginterfaces/vst/vsttypes.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg;
using namespace Steinberg::Vst;

using TSample = Steinberg::Vst::Sample64;

constexpr long UI_FRAME_RATE_MS = 40; // 40ms => 25 frames per seconds
//constexpr long UI_FRAME_RATE_MS = 250; // 4 per seconds for dev

constexpr int MAX_ARRAY_SIZE = 256; // size of LCD window
constexpr int MAX_LCD_INPUT_X = MAX_ARRAY_SIZE - 1;
constexpr double MAX_HISTORY_OFFSET = 1.0; // percentage

// the max will be accumulated for 5ms which is ~221 samples at 44100 sample rate
constexpr int ACCUMULATOR_BATCH_SIZE_IN_MS = 5;
constexpr int HISTORY_SIZE_IN_SECONDS = 30; // how long is the history in seconds
// the buffer size is independent of the sample rate since one sample in the buffer is always made of
// enough samples to fit ACCUMULATOR_BATCH_SIZE_IN_MS
constexpr int SAMPLE_BUFFER_SIZE = HISTORY_SIZE_IN_SECONDS * 1000 / ACCUMULATOR_BATCH_SIZE_IN_MS; // 6000 samples

// keeping track of the version of the state being saved so that it can be upgraded more easily later
constexpr uint16 PROCESSOR_STATE_VERSION = 1;
constexpr uint16 CONTROLLER_STATE_VERSION = 1;

/**
 * State of max level (hard clipping means above 0dB, soft clipping means above some defined threshold)
 */
enum EMaxLevelState
{
  kStateOk = 0,
  kStateSoftClipping = 1,
  kStateHardClipping = 2
};

}
}
}