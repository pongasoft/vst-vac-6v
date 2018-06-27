#include "MaxLevelView.h"
#include "DrawContext.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// MaxLevelState::setMaxLevel
///////////////////////////////////////////
void MaxLevelState::setMaxLevel(MaxLevel const &maxLevel)
{
  fMaxLevel = maxLevel;
  updateView();
}

///////////////////////////////////////////
// MaxLevelState::updateView
///////////////////////////////////////////
void MaxLevelState::updateView() const
{
  if(fView != nullptr)
  {
    fView->setDirty(true);
  }
}

///////////////////////////////////////////
// MaxLevelState::onMessage
///////////////////////////////////////////
void MaxLevelState::onMessage(Message const &message)
{
  MaxLevel maxLevel{};

  maxLevel.fLeftValue = message.getFloat(MAX_LEVEL_LEFT_VALUE_ATTR, -1);
  maxLevel.fRightValue = message.getFloat(MAX_LEVEL_RIGHT_VALUE_ATTR, -1);

  setMaxLevel(maxLevel);
}

///////////////////////////////////////////
// MaxLevelView::MaxLevelView
///////////////////////////////////////////
MaxLevelView::MaxLevelView(const CRect &size)
  : HistoryView(size)
{

}

///////////////////////////////////////////
// MaxLevelState::draw
///////////////////////////////////////////
void MaxLevelView::draw(CDrawContext *iContext)
{
  HistoryView::draw(iContext);

  if(fState == nullptr)
    return;

  TSample leftValue = fLCDLeftChannelParameter->getValue() ? fState->fMaxLevel.fLeftValue : 0.0;
  TSample rightValue = fLCDRightChannelParameter->getValue() ? fState->fMaxLevel.fRightValue : 0.0;
  TSample max = std::max(leftValue, rightValue);

  CColor fontColor = getNoDataColor();

  char text[256];
  if(max > 0)
  {
    fontColor = computeColor(fSoftClippingLevelParameter->getValue(), max);
    if(max >= Common::Sample64SilentThreshold)
      sprintf(text, "%+.2f", sampleToDb(max));
    else
      sprintf(text, "-oo");
  }
  else
  {
    sprintf(text, "---.--");
  }

  auto rdc = GUI::RelativeDrawContext{this, iContext};

  StringDrawContext sdc{};
  sdc.fHoriTxtAlign = kCenterText;
  sdc.fTextInset = {2, 2};
  sdc.fFontColor = fontColor;

  rdc.drawString(text, sdc);

}

///////////////////////////////////////////
// MaxLevelView::registerParameters
///////////////////////////////////////////
void MaxLevelView::registerParameters()
{
  HistoryView::registerParameters();

  fLCDLeftChannelParameter = registerBooleanParameter(EVAC6ParamID::kLCDLeftChannel);
  fLCDRightChannelParameter = registerBooleanParameter(EVAC6ParamID::kLCDRightChannel);
}

MaxLevelView::Creator __gMaxLevelViewCreator("pongasoft::MaxLevel", "pongasoft - Max Level");

}
}
}