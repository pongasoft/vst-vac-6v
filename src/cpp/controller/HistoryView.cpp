#include "HistoryView.h"
#include "../VAC6CIDs.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// HistoryView::computeColor
///////////////////////////////////////////
const CColor &HistoryView::computeColor(SoftClippingLevel iSofClippingLevel, double iSample) const
{
  CColor const &color =
    iSample > HARD_CLIPPING_LEVEL ? getLevelStateHardClippingColor() :
    iSample > iSofClippingLevel.getValueInSample() ? getLevelStateSoftClippingColor() :
    getLevelStateOkColor();

  return color;
}

///////////////////////////////////////////
// HistoryView::registerParameters
///////////////////////////////////////////
void HistoryView::registerParameters()
{
  CustomView::registerParameters();

  fSoftClippingLevelParameter = registerVSTParameter<SoftClippingLevelParameter>(EVAC6ParamID::kSoftClippingLevel);
  fMaxLevelModeParameter = registerVSTParameter<MaxLevelModeParameter>(EVAC6ParamID::kMaxLevelMode);
  fLCDLiveViewParameter = registerBooleanParameter(EVAC6ParamID::kLCDLiveView);
  fLCDInputXParameter = registerVSTParameter<LCDInputXParameter>(EVAC6ParamID::kLCDInputX);
}

///////////////////////////////////////////
// HistoryView::getMaxLevel
///////////////////////////////////////////
MaxLevel HistoryView::getMaxLevel() const
{
  if(fLCDLiveViewParameter->getValue())
  {
    // in live mode
    return fMaxLevelModeParameter->getValue() == kMaxInWindow ?
      fState->fMaxLevelInWindow :
      fState->fMaxLevelSinceReset;
  }
  else
  {
    // in pause mode
    int index = fLCDInputXParameter->getValue();

    MaxLevel left = fState->fLCDData.fLeftChannel.fOn ?
      MaxLevel{fState->fLCDData.fLeftChannel.fSamples[index], index} :
      MaxLevel{};
    MaxLevel right = fState->fLCDData.fRightChannel.fOn ?
      MaxLevel{fState->fLCDData.fRightChannel.fSamples[index], index} :
      MaxLevel{};

    return MaxLevel::computeMaxLevel(left, right);
  }
}

///////////////////////////////////////////
// HistoryState::onMessage
///////////////////////////////////////////
void HistoryState::onMessage(Message const &message)
{
  fLCDData.fWindowSizeInMillis = message.getInt(LCDDATA_WINDOW_SIZE_MS_ATTR, fLCDData.fWindowSizeInMillis);
  fLCDData.fLeftChannel.fOn = message.getBinary(LCDDATA_LEFT_SAMPLES_ATTR, fLCDData.fLeftChannel.fSamples, MAX_ARRAY_SIZE) > -1;
  fLCDData.fLeftChannel.fMaxLevelSinceReset = message.getFloat(LCDDATA_LEFT_MAX_LEVEL_SINCE_RESET_ATTR, -1);

  fLCDData.fRightChannel.fOn = message.getBinary(LCDDATA_RIGHT_SAMPLES_ATTR, fLCDData.fRightChannel.fSamples, MAX_ARRAY_SIZE) > -1;
  fLCDData.fRightChannel.fMaxLevelSinceReset = message.getFloat(LCDDATA_RIGHT_MAX_LEVEL_SINCE_RESET_ATTR, -1);

  LCDData::computeMaxLevels(fLCDData.fLeftChannel, fLCDData.fRightChannel, fMaxLevelInWindow, fMaxLevelSinceReset);
}


}
}
}