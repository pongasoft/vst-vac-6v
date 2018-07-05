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

  CColor fontColor = maxLevel.isUndefined() ?
                     getNoDataColor() :
                     computeColor(fSoftClippingLevelParameter->getValue(), maxLevel.fValue);

  auto rdc = GUI::RelativeDrawContext{this, iContext};

  StringDrawContext sdc{};
  sdc.fHoriTxtAlign = kCenterText;
  sdc.fTextInset = {2, 2};
  sdc.fFontColor = fontColor;

  rdc.drawString(maxLevel.toDbString(), sdc);
}

///////////////////////////////////////////
// MaxLevelView::setState
///////////////////////////////////////////
void MaxLevelView::setState(MaxLevelState *iState)
{
  fState = iState;
  if(iState)
    fHistoryState = iState->fHistoryState;
  else
    fHistoryState = nullptr;
}


MaxLevelSinceResetViewCreator __gMaxLevelSinceResetViewCreator("pongasoft::MaxLevelSinceReset", "pongasoft - Max Level - Since Reset");
MaxLevelInWindowViewCreator __gMaxLevelInWindowViewCreator("pongasoft::MaxLevelInWindow", "pongasoft - Max Level - In Window");
MaxLevelForSelectionViewCreator __gMaxLevelForSelectionViewCreator("pongasoft::MaxLevelForSelection", "pongasoft - Max Level - For Selection");
}
}
}