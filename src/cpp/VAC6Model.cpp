#include <pongasoft/VST/AudioUtils.h>
#include "VAC6Model.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

//------------------------------------------------------------------------
// LCDData::Channel::computeInWindowMaxLevel
//------------------------------------------------------------------------
MaxLevel LCDData::Channel::computeInWindowMaxLevel() const
{
  MaxLevel res = {};

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

//------------------------------------------------------------------------
// LCDData::Channel::computeSinceResetMaxLevel
//------------------------------------------------------------------------
MaxLevel LCDData::Channel::computeSinceResetMaxLevel() const
{
  MaxLevel oSinceReset = {fMaxLevelSinceReset, -1};

  if(fOn)
  {
    auto ptr = &fSamples[MAX_ARRAY_SIZE - 1];
    for(int i = MAX_ARRAY_SIZE - 1; i >= 0; i--)
    {
      TSample sample = *ptr--;
      if(sample == fMaxLevelSinceReset)
      {
        oSinceReset.fIndex = i;
        return oSinceReset;
      }
    }
  }

  return oSinceReset;
}

//------------------------------------------------------------------------
// MaxLevel::computeMaxLevel
//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
// MaxLevel::toDbString
//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
// VAC6::toDbString
//------------------------------------------------------------------------
std::string toDbString(TSample iSample, int iPrecision)
{
  if(iSample < 0)
    iSample = -iSample;

  std::ostringstream s;

  if(iSample >= VST::Sample64SilentThreshold)
  {
    s.precision(iPrecision);
    s.setf(std::ios::fixed);
    s << std::showpos << sampleToDb(iSample) << "dB";
  }
  else
    s << "-oo";
  return s.str();
}

//------------------------------------------------------------------------
// LCDZoomFactorXParamConverter::toString
//------------------------------------------------------------------------
std::string LCDZoomFactorXParamConverter::toString(const LCDZoomFactorXParamConverter::ParamType &iValue,
                                                   int32 iPrecision) const
{
  auto zoomInSeconds =
    Utils::mapValueDP(iValue,
                      0.0, 1.0,
                      static_cast<double>(HISTORY_SIZE_IN_SECONDS), ACCUMULATOR_BATCH_SIZE_IN_MS * MAX_ARRAY_SIZE / 1000.0);

  std::ostringstream s;
  s.precision(iPrecision);
  s.setf(std::ios::fixed);
  s << zoomInSeconds << "s";
  return s.str();
}

//------------------------------------------------------------------------
// HistoryData::getMaxLevelForSelection
//------------------------------------------------------------------------
MaxLevel HistoryData::getMaxLevelForSelection(int iLCDInputX) const
{
  // no selection
  if(iLCDInputX < 0)
    return MaxLevel{};

  MaxLevel res{-1, Utils::clampE(iLCDInputX, 0, MAX_LCD_INPUT_X)};

  TSample leftSample = fLCDData.fLeftChannel.fOn ? fLCDData.fLeftChannel.fSamples[res.fIndex] : -1;
  TSample rightSample = fLCDData.fRightChannel.fOn ? fLCDData.fRightChannel.fSamples[res.fIndex] : -1;

  res.fValue = std::max(leftSample, rightSample);

  return res;
}

//------------------------------------------------------------------------
// HistoryData::computeMaxLevels
//------------------------------------------------------------------------
void HistoryData::computeMaxLevels()
{
  fMaxLevelInWindow = MaxLevel::computeMaxLevel(fLCDData.fLeftChannel.computeInWindowMaxLevel(),
                                                fLCDData.fRightChannel.computeInWindowMaxLevel());
  fMaxLevelSinceReset = MaxLevel::computeMaxLevel(fLCDData.fLeftChannel.computeSinceResetMaxLevel(),
                                                  fLCDData.fRightChannel.computeSinceResetMaxLevel());
}

//------------------------------------------------------------------------
// HistoryData::readFromStream
//------------------------------------------------------------------------
tresult HistoryDataParamSerializer::readFromStream(IBStreamer &iStreamer, HistoryData &oValue) const
{
  {
    tresult res = LCDDataParamSerializer::readFromStream(iStreamer, oValue.fLCDData);
    if(res == kResultOk)
    {
      oValue.computeMaxLevels();
    }
    return res;
  }
}
}
}
}