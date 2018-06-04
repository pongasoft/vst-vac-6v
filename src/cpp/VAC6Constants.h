#pragma once

#include <pluginterfaces/vst/vsttypes.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg::Vst;

const Sample64 MIN_AUDIO_SAMPLE = 0.001;

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