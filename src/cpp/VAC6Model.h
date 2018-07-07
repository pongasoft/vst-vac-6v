#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <pluginterfaces/vst/ivstattributes.h>
#include <cmath>
#include "VAC6Constants.h"
#include "ZoomWindow.h"
#include "Parameter.h"

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

constexpr TSample DEFAULT_SOFT_CLIPPING_LEVEL = 0.50118723362; // dbToSample<TSample>(-6.0);
constexpr TSample HARD_CLIPPING_LEVEL = 1.0;
constexpr double MIN_SOFT_CLIPPING_LEVEL_DB = -24;
constexpr double MIN_VOLUME_DB = -60; // -60dB
constexpr TSample MIN_AUDIO_SAMPLE = 0.001; // dbToSample<TSample>(-60.0)
// zoom varies from 30s to 1.28s (5ms * 256=1.28s) and we want default to be 15s
constexpr double DEFAULT_ZOOM_FACTOR_X = 0.52228412256267409131;

using LCDHistoryOffsetParamConverter = Common::PercentParamConverter;
using LCDZoomFactorXParamConverter = Common::PercentParamConverter;

///////////////////////////////////
// LCDInputX
///////////////////////////////////

constexpr int LCD_INPUT_X_NOTHING_SELECTED = -1;

class LCDInputXParamConverter
{
private:
  using DVC = Common::DiscreteValueParamConverter<MAX_LCD_INPUT_X + 1>;
public:
  static inline ParamValue normalize(int const &iDiscreteValue)
  {
    return DVC::normalize(iDiscreteValue + 1);
  }

  static inline int denormalize(ParamValue iNormalizedValue)
  {
    return DVC::denormalize(iNormalizedValue) - 1;
  }
};

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

///////////////////////////////////////////
// // VAC6::toDbString
///////////////////////////////////////////
std::string toDbString(TSample iSample);

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

  inline ParamValue getNormalizedValue() const
  {
    return (MIN_SOFT_CLIPPING_LEVEL_DB - getValueInDb()) / MIN_SOFT_CLIPPING_LEVEL_DB;
  }

  static SoftClippingLevel denormalize(ParamValue value)
  {
    double sclIndB = -MIN_SOFT_CLIPPING_LEVEL_DB * value + MIN_SOFT_CLIPPING_LEVEL_DB;
    return SoftClippingLevel {dbToSample<TSample>(sclIndB)};
  }

  static ParamValue normalize(SoftClippingLevel const &iSoftClippingLevel)
  {
    return iSoftClippingLevel.getNormalizedValue();
  }

private:
  Sample64 fValueInSample;
};

///////////////////////////////////
// Gain
///////////////////////////////////
class Gain
{
public:
  static constexpr double Unity = 1.0;
  static constexpr double Factor = 0.7;

  constexpr explicit Gain(double iValue = Unity) : fValue{iValue} {}

  inline double getValue() const { return fValue; }
  inline double getValueInDb() const { return sampleToDb(fValue); }
  inline ParamValue getNormalizedValue() const { return normalize(*this); }

  /**
   * Gain uses an x^3 curve with 0.7 (Param Value) being unity gain
   */
  static Gain denormalize(ParamValue value)
  {
    if(std::fabs(value - Factor) < 1e-5)
      return Gain{};

    // gain = (value / 0.7) ^ 3
    double correctedGain = value / Factor;
    return Gain{correctedGain * correctedGain * correctedGain};
  }

  static ParamValue normalize(Gain const &iGain)
  {
    // value = (gain ^ 1/3) * 0.7
    return std::pow(iGain.fValue, 1.0/3) * Factor;
  }

private:
  double fValue;
};

constexpr Gain DEFAULT_GAIN = Gain{};

///////////////////////////////////
// MaxLevel
///////////////////////////////////
struct MaxLevel
{
  TSample fValue{-1};
  int fIndex{-1};

  bool isUndefined() const
  {
    return fValue < 0;
  }

  std::string toDbString() const;

  static MaxLevel computeMaxLevel(MaxLevel const &iLeftMaxLevel, MaxLevel const &iRightMaxLevel);
};


///////////////////////////////////
// LCDData
///////////////////////////////////
constexpr IAttributeList::AttrID LCDDATA_WINDOW_SIZE_MS_ATTR = "WSM";
constexpr IAttributeList::AttrID LCDDATA_LEFT_SAMPLES_ATTR = "LSA";
constexpr IAttributeList::AttrID LCDDATA_LEFT_MAX_LEVEL_SINCE_RESET_ATTR = "LML";
constexpr IAttributeList::AttrID LCDDATA_RIGHT_SAMPLES_ATTR = "RSA";
constexpr IAttributeList::AttrID LCDDATA_RIGHT_MAX_LEVEL_SINCE_RESET_ATTR = "RML";

struct LCDData
{
  struct Channel
  {
    bool fOn{true};
    TSample fSamples[MAX_ARRAY_SIZE]{};
    TSample fMaxLevelSinceReset{0};

    MaxLevel computeInWindowMaxLevel() const;
    MaxLevel computeSinceResetMaxLevel() const;
  };

  long fWindowSizeInMillis{0};

  Channel fLeftChannel;
  Channel fRightChannel;
};
}
}
}