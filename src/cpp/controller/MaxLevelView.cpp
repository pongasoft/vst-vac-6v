#include "MaxLevelView.h"
#include "../AudioUtils.h"

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

  maxLevel.fSoftClippingLevel = SoftClippingLevel{message.getFloat(MAX_LEVEL_SOFT_CLIPPING_LEVEL_ATTR,
                                                                   fMaxLevel.fSoftClippingLevel.getValueInSample())};
  maxLevel.fLeftValue = message.getFloat(MAX_LEVEL_LEFT_VALUE_ATTR, -1);
  maxLevel.fRightValue = message.getFloat(MAX_LEVEL_RIGHT_VALUE_ATTR, -1);

  setMaxLevel(maxLevel);
}

///////////////////////////////////////////
// MaxLevelView::MaxLevelView
///////////////////////////////////////////
MaxLevelView::MaxLevelView(const CRect &size, IControlListener *listener, int32_t tag, CBitmap *pBackground)
  : HistoryView(size, listener, tag, pBackground)
{

}

///////////////////////////////////////////
// MaxLevelState::draw
///////////////////////////////////////////
void MaxLevelView::draw(CDrawContext *iContext)
{
  CustomDisplayView::draw(iContext);

  if(fState == nullptr)
    return;

  TSample max = std::max(fState->fMaxLevel.fLeftValue, fState->fMaxLevel.fRightValue);

  CColor fontColor = getNoDataColor();

  char text[256];
  if(max > 0)
  {
    fontColor = computeColor(fState->fMaxLevel.fSoftClippingLevel, max);
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

MaxLevelViewCreator __gMaxLevelViewCreator("pongasoft::MaxLevel", "pongasoft - Max Level");

}
}
}