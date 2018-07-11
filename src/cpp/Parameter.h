#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <cmath>
#include <algorithm>
#include <pluginterfaces/base/ustring.h>
#include "AudioUtils.h"

namespace pongasoft {
namespace VST {
namespace Common {

using namespace Steinberg::Vst;

//template<typename T>
//class ParamConverter
//{
//public:
//  inline static ParamValue normalize(T const &iValue);
//  inline static T denormalize(ParamValue iNormalizedValue);
//};

class RawParamConverter
{
public:
  using ParamType = ParamValue;

  inline static ParamValue normalize(ParamValue const &iValue)
  {
    return iValue;
  }

  inline static ParamValue denormalize(ParamValue iNormalizedValue)
  {
    return iNormalizedValue;
  }

  inline static void toString(ParamValue const &iValue, String128 iString, int32 iPrecision)
  {
    Steinberg::UString wrapper(iString, str16BufferSize (String128));
    if(!wrapper.printFloat(iValue, iPrecision))
      iString[0] = 0;
  }
};


class BooleanParamConverter
{
public:
  using ParamType = bool;

  inline static ParamValue normalize(bool const &iValue)
  {
    return iValue ? 1.0 : 0;
  }

  inline static bool denormalize(ParamValue iNormalizedValue)
  {
    return iNormalizedValue >= 0.5;
  }

  inline static void toString(bool const &iValue, String128 iString, int32 /* iPrecision */)
  {
    Steinberg::UString wrapper(iString, str16BufferSize(String128));
    if(iValue)
      wrapper.assign(STR16("On"));
    else
      wrapper.assign(STR16("Off"));
  }
};

/**
 * A trivial percent converter.
 */
class PercentParamConverter
{
public:
  using ParamType = double;

  inline static ParamValue normalize(double const &iValue)
  {
    return clamp(iValue, 0.0, 1.0);
  }

  inline static double denormalize(ParamValue iNormalizedValue)
  {
    return clamp(iNormalizedValue, 0.0, 1.0);
  }

  inline static void toString(ParamType const &iValue, String128 iString, int32 iPrecision)
  {
    Steinberg::UString wrapper(iString, str16BufferSize (String128));
    wrapper.printFloat(iValue * 100, iPrecision);
    wrapper.append(STR16("%"));
  }
};

template<int StepCount>
class DiscreteValueParamConverter
{
public:
  using ParamType = int;

  static inline ParamValue normalize(int const &iDiscreteValue)
  {
    auto value = clamp(iDiscreteValue, 0, StepCount);
    return value / static_cast<double>(StepCount);
  }

  static inline int denormalize(ParamValue iNormalizedValue)
  {
    // ParamValue must remain within its bounds
    auto value = clamp(iNormalizedValue, 0.0, 1.0);
    return static_cast<int>(std::floor(std::min(static_cast<ParamValue>(StepCount),
                                                value * (StepCount + 1))));
  }

  inline static void toString(ParamType const &iValue, String128 iString, int32 /* iPrecision */)
  {
    Steinberg::UString wrapper(iString, str16BufferSize (String128));
    if(!wrapper.printInt(iValue))
      iString[0] = 0;
  }
};

/*
 * Simple function to add a single parameter change at position 0 (which is the vast majority of cases) */
inline tresult addOutputParameterChange(ProcessData &data, ParamID iParamID, ParamValue iNormalizedValue)
{
  IParameterChanges* outParamChanges = data.outputParameterChanges;
  if(outParamChanges != nullptr)
  {
    int32 index = 0;
    auto paramQueue = outParamChanges->addParameterData(iParamID, index);
    if(paramQueue != nullptr)
    {
      int32 index2 = 0;
      return paramQueue->addPoint(0, iNormalizedValue, index2);
    }
  }

  return kResultFalse;
}
}
}
}