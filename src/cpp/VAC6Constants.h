#pragma once

#include <pluginterfaces/vst/vsttypes.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg::Vst;

using TSample = Steinberg::Vst::Sample64;

//constexpr long UI_FRAME_RATE_MS = 40; // 40ms => 25 frames per seconds
constexpr long UI_FRAME_RATE_MS = 250; // 4 per seconds for dev

constexpr int MAX_ARRAY_SIZE = 256; // size of LCD window
constexpr int MAX_INPUT_PAGE_OFFSET = MAX_ARRAY_SIZE - 1;

// the max will be accumulated for 1.5ms which is ~67 samples at 44100 sample rate
constexpr float BATCH_SIZE_IN_MS = 1.5;
constexpr int HISTORY_SIZE_IN_SECONDS = 10; // how long is the history in seconds

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