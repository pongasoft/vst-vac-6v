#include "MaxLevelView.h"
#include "DrawContext.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

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
  HistoryState::onMessage(message);

  updateView();
}

///////////////////////////////////////////
// MaxLevelView::MaxLevelView
///////////////////////////////////////////
MaxLevelView::MaxLevelView(const CRect &size)
  : HistoryView(size)
{

}

///////////////////////////////////////////
// MaxLevelView::draw
///////////////////////////////////////////
void MaxLevelView::draw(CDrawContext *iContext)
{
  HistoryView::draw(iContext);

  if(fState == nullptr)
    return;

  auto maxLevel = getMaxLevel();

  TSample max = maxLevel.fValue;

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
// MaxLevelView::setState
///////////////////////////////////////////
void MaxLevelView::setState(MaxLevelState *iState)
{
  HistoryView::setState((iState));
  fState = iState;
}


MaxLevelView::Creator __gMaxLevelViewCreator("pongasoft::MaxLevel", "pongasoft - Max Level");

}
}
}