#pragma once

#include <pluginterfaces/vst/vsttypes.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg::Vst;

const Sample64 MIN_AUDIO_SAMPLE = 0.001;

enum EMaxLevelState
{
  kStateOk = 0,
  kStateSoftClipping = 1,
  kStateHardClipping = 2
};
}
}
}