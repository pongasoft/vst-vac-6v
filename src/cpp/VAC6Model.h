#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <pluginterfaces/vst/ivstattributes.h>
#include <pongasoft/VST/ParamConverters.h>
#include <cmath>
#include <sstream>
#include "VAC6Constants.h"
#include "ZoomWindow.h"

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
constexpr bool DEFAULT_GAIN_FILTER = true;

using LCDHistoryOffsetParamConverter = PercentParamConverter;

///////////////////////////////////
// LCDZoomFactorXParamConverter
///////////////////////////////////

// zoom varies from 30s to 1.28s (5ms * 256=1.28s) and we want default to be 15s
constexpr double DEFAULT_ZOOM_FACTOR_X = 0.52228412256267409131;

class LCDZoomFactorXParamConverter
{
private:
  using PPC = PercentParamConverter;

public:
  using ParamType = PPC::ParamType;

  inline static ParamValue normalize(ParamType const &iValue)
  {
    return PPC::normalize(iValue);
  }

  inline static ParamType denormalize(ParamValue iNormalizedValue)
  {
    return PPC::denormalize(iNormalizedValue);
  }

  static std::string toString(ParamType const &iValue, int32 iPrecision);

  inline static void toString(ParamType const &iValue, String128 iString, int32 iPrecision)
  {
    auto s = toString(iValue, iPrecision);
    Steinberg::UString wrapper(iString, str16BufferSize (String128));
    wrapper.fromAscii(s.c_str());
  }
};


///////////////////////////////////
// LCDInputX
///////////////////////////////////

constexpr int LCD_INPUT_X_NOTHING_SELECTED = -1;

class LCDInputXParamConverter
{
private:
  using DVC = DiscreteValueParamConverter<MAX_LCD_INPUT_X + 1>;
public:
  using ParamType = DVC::ParamType;

  static inline ParamValue normalize(int const &iDiscreteValue)
  {
    return DVC::normalize(iDiscreteValue + 1);
  }

  static inline int denormalize(ParamValue iNormalizedValue)
  {
    return DVC::denormalize(iNormalizedValue) - 1;
  }

  inline static void toString(ParamType const &iValue, String128 iString, int32 iPrecision)
  {
    DVC::toString(iValue, iString, iPrecision);
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
std::string toDbString(TSample iSample, int iPrecision = 2);

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

private:
  Sample64 fValueInSample;
};

///////////////////////////////////
// SoftClippingLevelParamConverter
///////////////////////////////////
class SoftClippingLevelParamConverter
{
public:
  using ParamType = SoftClippingLevel;

  static inline ParamValue normalize(ParamType const &iValue)
  {
    return iValue.getNormalizedValue();
  }

  static inline ParamType denormalize(ParamValue iNormalizedValue)
  {
    double sclIndB = -MIN_SOFT_CLIPPING_LEVEL_DB * iNormalizedValue + MIN_SOFT_CLIPPING_LEVEL_DB;
    return SoftClippingLevel {dbToSample<TSample>(sclIndB)};
  }

  inline static void toString(ParamType const &iValue, String128 iString, int32 iPrecision)
  {
    auto s = toDbString(iValue.getValueInSample());
    Steinberg::UString wrapper(iString, str16BufferSize (String128));
    wrapper.fromAscii(s.c_str());
  }
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
  inline ParamValue getNormalizedValue() const
  {
    // value = (gain ^ 1/3) * 0.7
    return std::pow(fValue, 1.0/3) * Factor;
  }

private:
  double fValue;
};

constexpr Gain DEFAULT_GAIN = Gain{};

class GainParamConverter
{
public:
  using ParamType = Gain;

  /**
   * Gain uses an x^3 curve with 0.7 (Param Value) being unity gain
   */
  static Gain denormalize(ParamValue value)
  {
    if(std::fabs(value - Gain::Factor) < 1e-5)
      return Gain{};

    // gain = (value / 0.7) ^ 3
    double correctedGain = value / Gain::Factor;
    return Gain{correctedGain * correctedGain * correctedGain};
  }

  // normalize
  static ParamValue normalize(Gain const &iGain)
  {
    return iGain.getNormalizedValue();
  }

  // toString
  inline static void toString(ParamType const &iValue, String128 iString, int32 iPrecision)
  {
    auto s = toDbString(iValue.getValue(), iPrecision);
    Steinberg::UString wrapper(iString, str16BufferSize (String128));
    wrapper.fromAscii(s.c_str());
  }
};

///////////////////////////////////
// FilteredGain - Gain which changes slowly to avoid nasty effects
///////////////////////////////////
class FilteredGain
{
public:
  explicit FilteredGain(double iValue = Gain::Unity, bool iFilterOn = true) :
    fValue{iValue},
    fTargetValue{fValue},
    fFilterOn{iFilterOn}
  {}

  // adjust should be called every frame
  bool adjust()
  {
    if(fTargetValue == fValue)
      return false;

    double previousValue = fValue;
    if(!fFilterOn)
    {
      fValue = fTargetValue;
    }
    else
    {
      // Filter changes to avoid nasty sounds.
      if(std::abs(fValue - fTargetValue) < 0.01f)
      {
        fValue = fTargetValue;
      }
      else
      {
        fValue += (fTargetValue - fValue) / 100.0f;
        //DLOG_F(INFO, "gain adjusted from %f to %f with target %f", previousValue, fValue, fTargetValue);
      }
    }

    return previousValue != fValue;
  }

  inline void setTargetValue(double iTargetValue) { fTargetValue = iTargetValue; }
  inline double getValue() const { return fValue; }
  inline void setFilterOn(bool iFilterOn) { fFilterOn = iFilterOn; }

private:
  double fValue;
  double fTargetValue;
  bool fFilterOn;
};

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

  std::string toDbString(int iPrecision = 2) const;

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