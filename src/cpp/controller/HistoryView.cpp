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
}

///////////////////////////////////////////
// HistoryView::getMaxLevel
///////////////////////////////////////////
MaxLevel HistoryView::getMaxLevel() const
{
  return fMaxLevelModeParameter->getValue() == kMaxInWindow ? fHistoryState->fMaxLevelInWindow : fHistoryState->fMaxLevelSinceReset;
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