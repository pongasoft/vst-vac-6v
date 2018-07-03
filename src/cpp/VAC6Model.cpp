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
}
}
}