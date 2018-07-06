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
  fLCDInputXParameter = registerVSTParameter<LCDInputXParameter>(EVAC6ParamID::kLCDInputX);
}

///////////////////////////////////////////
// HistoryView::getMaxLevelSinceReset
///////////////////////////////////////////
MaxLevel HistoryView::getMaxLevelSinceReset() const
{
  return fHistoryState->fMaxLevelSinceReset;
}

///////////////////////////////////////////
// HistoryView::getMaxLevelInWindow
///////////////////////////////////////////
MaxLevel HistoryView::getMaxLevelInWindow() const
{
  return fHistoryState->fMaxLevelInWindow;
}

///////////////////////////////////////////
// HistoryView::getMaxLevelForSelection
///////////////////////////////////////////
MaxLevel HistoryView::getMaxLevelForSelection() const
{
  // no selection
  if(fLCDInputXParameter->getValue() < 0)
    return MaxLevel{};

  MaxLevel res{-1, clampE(fLCDInputXParameter->getValue(), 0, MAX_LCD_INPUT_X)};

  LCDData &lcdData = fHistoryState->fLCDData;

  TSample leftSample = lcdData.fLeftChannel.fOn ? lcdData.fLeftChannel.fSamples[res.fIndex] : -1;
  TSample rightSample = lcdData.fRightChannel.fOn ? lcdData.fRightChannel.fSamples[res.fIndex] : -1;

  res.fValue = std::max(leftSample, rightSample);

  return res;
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

  fMaxLevelInWindow = MaxLevel::computeMaxLevel(fLCDData.fLeftChannel.computeInWindowMaxLevel(),
                                                fLCDData.fRightChannel.computeInWindowMaxLevel());
  fMaxLevelSinceReset = MaxLevel::computeMaxLevel(fLCDData.fLeftChannel.computeSinceResetMaxLevel(),
                                                  fLCDData.fRightChannel.computeSinceResetMaxLevel());
}


}
}
}