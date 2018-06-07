#pragma once

#include <pluginterfaces/vst/vsttypes.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg::Vst;

constexpr Sample64 MIN_AUDIO_SAMPLE = 0.001;
//constexpr long UI_FRAME_RATE_MS = 40; // 40ms => 25 frames per seconds
constexpr long UI_FRAME_RATE_MS = 250; // 4 per seconds for dev

/**
 * State of max level (hard clipping means above 0dB, soft clipping means above some defined threshold)
 */
enum EMaxLevelState
{
  kStateOk = 0,
  kStateSoftClipping = 1,
  kStateHardClipping = 2
};

/**
 * Encapsulates the concept of max level
 */
struct MaxLevel
{
  Sample64 fValue;
  EMaxLevelState fState;
};

}
}
}