#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <cmath>

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

// convert a soft clipping level [0,1.0] into a soft clipping volume level
// 1.0  =>  0dB  => (10^0/20   => 1.0 sample value)
// 0.75 => -6dB  => (10^-6/20  => 0.5012 sample value)
// 0.0  => -24dB => (10^-24/20 => 0.06 sample value)
// => scl = A * x + B in dB => 10 ^ scl/20 in sample
constexpr Sample64 MIN_SOFT_CLIPPING_LEVEL = -24;
inline Sample64 toSoftClippingLevel(ParamValue value)
{
  Sample64 sclIndB = -MIN_SOFT_CLIPPING_LEVEL * value + MIN_SOFT_CLIPPING_LEVEL;
  return static_cast<Sample64>(std::pow(10.0, sclIndB / 20.0));
}

inline ParamValue fromSoftLevelClipping(Sample64 softLevelClipping)
{
  Sample64 sclIndB = 20.0 * std::log(softLevelClipping) / std::log(10.0);
  return (MIN_SOFT_CLIPPING_LEVEL - sclIndB) / MIN_SOFT_CLIPPING_LEVEL;
}

}
}
}