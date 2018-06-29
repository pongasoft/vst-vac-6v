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
constexpr int HISTORY_SIZE_IN_SECONDS = 10; // how long is the history in seconds

// the number of steps for max level auto reset (0 means no auto reset. 1-10 represents how many seconds)
constexpr uint32 MAX_LEVEL_AUTO_RESET_STEP_COUNT = 10;

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