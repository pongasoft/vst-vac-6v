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

template<typename T>
inline T denormalizeDiscreteValue(T iStepCount, ParamValue iNormalizedValue)
{
  return static_cast<T>(std::floor(std::min(static_cast<ParamValue>(iStepCount), iNormalizedValue * (iStepCount + 1))));
}

inline ParamValue normalizeBoolValue(bool iValue)
{
  return iValue ? 1.0 : 0;
}

inline bool denormalizeBoolValue(ParamValue iNormalizedValue)
{
  return iNormalizedValue > 0;
}

}
}
}