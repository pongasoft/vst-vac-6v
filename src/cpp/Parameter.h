#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <cmath>
#include <algorithm>
#include "AudioUtils.h"

namespace pongasoft {
namespace VST {
namespace Common {

using namespace Steinberg::Vst;

template<int StepCount>
inline ParamValue normalizeDiscreteValue(int iDiscreteValue)
{
  iDiscreteValue = clamp(iDiscreteValue, 0, StepCount);
  return iDiscreteValue / static_cast<double>(StepCount);
}

template<int StepCount>
inline int denormalizeDiscreteValue(ParamValue iNormalizedValue)
{
  return static_cast<int>(std::floor(std::min(static_cast<ParamValue>(StepCount), iNormalizedValue * (StepCount + 1))));
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