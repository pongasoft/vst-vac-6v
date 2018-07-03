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
  fLCDLiveViewParameter = registerBooleanParameter(EVAC6ParamID::kLCDLiveView);
  fLCDInputXParameter = registerVSTParameter<LCDInputXParameter>(EVAC6ParamID::kLCDInputX);
}

///////////////////////////////////////////
// HistoryView::getMaxLevel
///////////////////////////////////////////
MaxLevel HistoryView::getMaxLevel() const
{
  return fHistoryState->fMaxLevel;
}

///////////////////////////////////////////
// HistoryState::onMessage
///////////////////////////////////////////
void HistoryState::onMessage(Message const &message)
{
  fLCDData.fWindowSizeInMillis = message.getInt(LCDDATA_WINDOW_SIZE_MS_ATTR, fLCDData.fWindowSizeInMillis);
  fLCDData.fLCDInputX = static_cast<int>(message.getInt(LCDDATA_LCD_INPUT_X_ATTR, fLCDData.fLCDInputX));
  fLCDData.fMaxLevelMode = static_cast<MaxLevelMode>(message.getInt(LCDDATA_LCD_INPUT_X_ATTR, fLCDData.fMaxLevelMode));
  fLCDData.fLeftChannel.fOn = message.getBinary(LCDDATA_LEFT_SAMPLES_ATTR, fLCDData.fLeftChannel.fSamples, MAX_ARRAY_SIZE) > -1;
  fLCDData.fLeftChannel.fMaxLevelSinceReset = message.getFloat(LCDDATA_LEFT_MAX_LEVEL_SINCE_RESET_ATTR, -1);

  fLCDData.fRightChannel.fOn = message.getBinary(LCDDATA_RIGHT_SAMPLES_ATTR, fLCDData.fRightChannel.fSamples, MAX_ARRAY_SIZE) > -1;
  fLCDData.fRightChannel.fMaxLevelSinceReset = message.getFloat(LCDDATA_RIGHT_MAX_LEVEL_SINCE_RESET_ATTR, -1);

  // in live view mode, processing sets this to an invalid value (-1)
  if(fLCDData.fLCDInputX == -1)
  {
    if(fMaxLevelMode == kMaxInWindow)
    {
      fMaxLevel = MaxLevel::computeMaxLevel(fLCDData.fLeftChannel.computeInWindowMaxLevel(),
                                            fLCDData.fRightChannel.computeInWindowMaxLevel());
    }
    else
    {
      fMaxLevel = MaxLevel::computeMaxLevel(fLCDData.fLeftChannel.computeSinceResetMaxLevel(),
                                            fLCDData.fRightChannel.computeSinceResetMaxLevel());

    }
  }
  else
  {
    // in pause mode processing sets this value to the selected point
    int index = fLCDData.fLCDInputX;

    DCHECK_F(index >= 0 && index < MAX_ARRAY_SIZE);

    MaxLevel left = fLCDData.fLeftChannel.fOn ?
                    MaxLevel{fLCDData.fLeftChannel.fSamples[index], index} :
                    MaxLevel{};
    MaxLevel right = fLCDData.fRightChannel.fOn ?
                     MaxLevel{fLCDData.fRightChannel.fSamples[index], index} :
                     MaxLevel{};

    fMaxLevel = MaxLevel::computeMaxLevel(left, right);
  }
}


}
}
}