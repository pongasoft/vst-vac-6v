#include "VAC6Model.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////
// LCDData::Channel::computeInWindowMaxLevel
///////////////////////////////////
MaxLevel LCDData::Channel::computeInWindowMaxLevel() const
{
  MaxLevel res = {};

  // TODO optimization: reverse the loop and return right away when found
  if(fOn)
  {
    auto ptr = &fSamples[0];
    for(int i = 0; i < MAX_ARRAY_SIZE; i++)
    {
      TSample sample = *ptr++;
      if(sample > res.fValue)
      {
        res.fValue = sample;
        res.fIndex = i;
      }
    }
  }
  return res;
}

///////////////////////////////////
// LCDData::Channel::computeSinceResetMaxLevel
///////////////////////////////////
MaxLevel LCDData::Channel::computeSinceResetMaxLevel() const
{
  MaxLevel oSinceReset = {fMaxLevelSinceReset, -1};

  // TODO optimization: reverse the loop and return right away when found

  if(fOn)
  {
    auto ptr = &fSamples[0];
    for(int i = 0; i < MAX_ARRAY_SIZE; i++)
    {
      TSample sample = *ptr++;
      if(sample == fMaxLevelSinceReset)
      {
        oSinceReset.fIndex = i;
      }
    }
  }

  return oSinceReset;
}

///////////////////////////////////
// MaxLevel::computeMaxLevel
///////////////////////////////////
MaxLevel MaxLevel::computeMaxLevel(MaxLevel const &iLeftMaxLevel, MaxLevel const &iRightMaxLevel)
{
  MaxLevel res;

  if(iLeftMaxLevel.fValue == iRightMaxLevel.fValue)
  {
    res.fValue = iLeftMaxLevel.fValue;
    res.fIndex = std::max(iLeftMaxLevel.fIndex, iRightMaxLevel.fIndex);
  }
  else
  {
    if(iLeftMaxLevel.fValue < iRightMaxLevel.fValue)
    {
      res.fValue = iRightMaxLevel.fValue;
      res.fIndex = iRightMaxLevel.fIndex;
    }
    else
    {
      res.fValue = iLeftMaxLevel.fValue;
      res.fIndex = iLeftMaxLevel.fIndex;
    }
  }

  return res;
}

///////////////////////////////////
// MaxLevel::toDbString
///////////////////////////////////
std::string MaxLevel::toDbString(int iPrecision) const
{
  if(isUndefined())
  {
    return "---.--";
  }
  else
  {
    return VAC6::toDbString(fValue, iPrecision);
  }
}

///////////////////////////////////
// VAC6::toDbString
///////////////////////////////////
std::string toDbString(TSample iSample, int iPrecision)
{
  if(iSample < 0)
    iSample = -iSample;

  std::ostringstream s;

  if(iSample >= Common::Sample64SilentThreshold)
  {
    s.precision(iPrecision);
    s.setf(std::ios::fixed);
    s << std::showpos << sampleToDb(iSample) << "dB";
  }
  else
    s << "-oo";
  return s.str();
}

///////////////////////////////////
// LCDZoomFactorXParamConverter::toString
///////////////////////////////////
std::string LCDZoomFactorXParamConverter::toString(const LCDZoomFactorXParamConverter::ParamType &iValue,
                                                   int32 iPrecision)
{
  auto lerpInSeconds = Utils::Lerp<double>(HISTORY_SIZE_IN_SECONDS,
                                           ACCUMULATOR_BATCH_SIZE_IN_MS * MAX_ARRAY_SIZE / 1000.0);
  std::ostringstream s;
  s.precision(iPrecision);
  s.setf(std::ios::fixed);
  s << lerpInSeconds.computeY(iValue) << "s";
  return s.str();
}
}
}
}