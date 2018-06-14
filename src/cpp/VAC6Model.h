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
  return std::log10(valueInSample) * 20.0;
}

constexpr IAttributeList::AttrID MAX_LEVEL_SOFT_CLIPPING_LEVEL_ATTR = "SCL";
constexpr IAttributeList::AttrID MAX_LEVEL_LEFT_VALUE_ATTR = "LValue";
constexpr IAttributeList::AttrID MAX_LEVEL_RIGHT_VALUE_ATTR = "RValue";

constexpr TSample DEFAULT_SOFT_CLIPPING_LEVEL = 0.50118723362; // dbToSample<TSample>(-6.0);
constexpr TSample HARD_CLIPPING_LEVEL = 1.0;
constexpr double MIN_SOFT_CLIPPING_LEVEL_DB = -24;
constexpr double MIN_VOLUME_DB = -60; // -60dB
constexpr TSample MIN_AUDIO_SAMPLE = 0.001; // dbToSample<TSample>(-60.0)
constexpr double DEFAULT_ZOOM_FACTOR_X = 0.5;
constexpr long DEFAULT_MAX_LEVEL_RESET_IN_SECONDS = 5;

///////////////////////////////////////////
// toDisplayValue
///////////////////////////////////////////
inline double toDisplayValue(TSample iSample, double iHeight)
{
  auto displayVolume = -((sampleToDb(iSample) / MIN_VOLUME_DB) - 1.0);
  return displayVolume * iHeight;
}

#ifndef NDEBUG
///////////////////////////////////////////
// fromDisplayValue
// inverse of toDisplayValue used for dev/debug only
///////////////////////////////////////////
inline TSample fromDisplayValue(double iDisplayValue, double iHeight)
{
  return dbToSample<double>((1.0 - (iDisplayValue / iHeight)) * MIN_VOLUME_DB);
}
#endif

///////////////////////////////////
// SoftClippingLevel
///////////////////////////////////
class SoftClippingLevel
{
public:
  explicit SoftClippingLevel(TSample valueInSample = DEFAULT_SOFT_CLIPPING_LEVEL) : fValueInSample(valueInSample)
  {}

  inline TSample getValueInSample() const { return fValueInSample; }

  inline double getValueInDb() const { return sampleToDb(getValueInSample()); }

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
    return SoftClippingLevel {dbToSample<TSample>(sclIndB)};
  }

private:
  Sample64 fValueInSample;
};

///////////////////////////////////
// MaxLevel
///////////////////////////////////
struct MaxLevel
{
  SoftClippingLevel fSoftClippingLevel{};
  TSample fLeftValue{-1};
  TSample fRightValue{-1};
};


///////////////////////////////////
// LCDData
///////////////////////////////////
constexpr IAttributeList::AttrID LCDDATA_SOFT_CLIPPING_LEVEL_ATTR = "SCL";
constexpr IAttributeList::AttrID LCDDATA_LEFT_SAMPLES_ATTR = "LSamples";
constexpr IAttributeList::AttrID LCDDATA_RIGHT_SAMPLES_ATTR = "RSamples";

struct LCDData
{
  SoftClippingLevel fSoftClippingLevel{};
  TSample fLeftSamples[MAX_ARRAY_SIZE]{};
  TSample fRightSamples[MAX_ARRAY_SIZE]{};
};

}
}
}