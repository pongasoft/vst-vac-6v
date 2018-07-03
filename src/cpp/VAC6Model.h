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
constexpr double DEFAULT_ZOOM_FACTOR_X = 0.5;

using LCDInputXParamConverter = Common::DiscreteValueParamConverter<MAX_LCD_INPUT_X>;
using LCDHistoryOffsetParamConverter = Common::PercentParamConverter;
using LCDZoomFactorXParamConverter = Common::PercentParamConverter;

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
// MaxLevelMode
///////////////////////////////////
enum MaxLevelMode
{
  kMaxSinceReset = 0,
  kMaxInWindow = 1
};

constexpr MaxLevelMode DEFAULT_MAX_LEVEL_MODE = MaxLevelMode::kMaxSinceReset;

class MaxLevelModeParamConverter
{
private:
  using DVC = Common::DiscreteValueParamConverter<1>;

public:
  static inline ParamValue normalize(MaxLevelMode const &iMaxLevelVode)
  {
    return DVC::normalize(iMaxLevelVode);
  }

  static inline MaxLevelMode denormalize(ParamValue iNormalizedValue)
  {
    int discreteValue = DVC::denormalize(iNormalizedValue);
    if(discreteValue == 0)
      return kMaxSinceReset;
    else
      return kMaxInWindow;
  }
};

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

  inline ParamValue getNormalizedParam() const
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
    return iSoftClippingLevel.getNormalizedParam();
  }

private:
  Sample64 fValueInSample;
};

///////////////////////////////////
// MaxLevel
///////////////////////////////////
struct MaxLevel
{
  TSample fValue{0};
  int fIndex{-1};

  static MaxLevel computeMaxLevel(MaxLevel const &iLeftMaxLevel, MaxLevel const &iRightMaxLevel);
};


///////////////////////////////////
// LCDData
///////////////////////////////////
constexpr IAttributeList::AttrID LCDDATA_WINDOW_SIZE_MS_ATTR = "WSM";
constexpr IAttributeList::AttrID LCDDATA_LCD_INPUT_X_ATTR = "LIX";
constexpr IAttributeList::AttrID LCDDATA_MAX_LEVEL_MODE_ATTR = "MLM";
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
  int fLCDInputX{-1};
  MaxLevelMode fMaxLevelMode{DEFAULT_MAX_LEVEL_MODE};

  Channel fLeftChannel;
  Channel fRightChannel;

  bool isLiveView() const
  {
    return fLCDInputX == -1;
  }
};
}
}
}