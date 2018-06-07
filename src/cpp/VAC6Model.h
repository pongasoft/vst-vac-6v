#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <cmath>

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg::Vst;


///////////////////////////////////
// dbToSample
///////////////////////////////////
template<typename SampleType>
inline SampleType dbToSample(double valueInDb)
{
  return static_cast<SampleType>(std::pow(10.0, valueInDb / 20.0));
}

///////////////////////////////////
// sampleToDb
///////////////////////////////////
template<typename SampleType>
inline double sampleToDb(SampleType valueInSample)
{
  return 20.0 * std::log(valueInSample) / std::log(10.0);
}

const Sample64 DEFAULT_SOFT_CLIPPING_LEVEL = dbToSample<Sample64>(-6.0);
constexpr double MIN_SOFT_CLIPPING_LEVEL_DB = -24;

///////////////////////////////////
// SoftClippingLevel
///////////////////////////////////
class SoftClippingLevel
{
public:
  SoftClippingLevel(Sample64 valueInSample = DEFAULT_SOFT_CLIPPING_LEVEL) : fValueInSample(valueInSample)
  {}

  inline Sample64 getValueInSample()
  { return fValueInSample; }

  inline double getValueInDb()
  { return sampleToDb(getValueInSample()); }

// convert a soft clipping level [0,1.0] into a soft clipping volume level
// 1.0  =>  0dB  => (10^0/20   => 1.0 sample value)
// 0.75 => -6dB  => (10^-6/20  => 0.5012 sample value)
// 0.0  => -24dB => (10^-24/20 => 0.06 sample value)
// => scl = A * x + B in dB => 10 ^ scl/20 in sample

  inline ParamValue getNormalizedParam()
  {
    return (MIN_SOFT_CLIPPING_LEVEL_DB - getValueInDb()) / MIN_SOFT_CLIPPING_LEVEL_DB;
  }

  static SoftClippingLevel fromNormalizedParam(ParamValue value)
  {
    double sclIndB = -MIN_SOFT_CLIPPING_LEVEL_DB * value + MIN_SOFT_CLIPPING_LEVEL_DB;
    return SoftClippingLevel {dbToSample<Sample64>(sclIndB)};
  }

private:
  Sample64 fValueInSample;
};

}
}
}