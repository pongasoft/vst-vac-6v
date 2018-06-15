#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <cmath>
#include <algorithm>

namespace pongasoft {
namespace VST {
namespace Common {

using namespace Steinberg::Vst;

inline ParamValue normalizeDiscreteValue(int iStepCount, int iDiscreteValue)
{
  return iDiscreteValue / static_cast<double>(iStepCount);
}

inline int denormalizeDiscreteValue(int iStepCount, ParamValue iNormalizedValue)
{
  return static_cast<int>(std::floor(std::min(static_cast<ParamValue>(iStepCount), iNormalizedValue * (iStepCount + 1))));
}

}
}
}