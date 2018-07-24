#include <pongasoft/VST/GUI/DrawContext.h>
#include "MaxLevelView.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// MaxLevelState::onMessage
///////////////////////////////////////////
void MaxLevelState::onMessage(Message const &message)
{
  updateView();
}

///////////////////////////////////////////
// MaxLevelState::onMessage
///////////////////////////////////////////
MaxLevel MaxLevelState::getMaxLevel(int iLCDInputX) const
{
  switch(fType)
  {
    case Type::kForSelection:
      return fHistoryState->getMaxLevelForSelection(iLCDInputX);

    case Type::kSinceReset:
      return fHistoryState->fMaxLevelSinceReset;

    case Type::kInWindow:
      return fHistoryState->fMaxLevelInWindow;
  }
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
  sdc.fFont = fFont;

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

///////////////////////////////////////////
// MaxLevelView::getMaxLevel
///////////////////////////////////////////
MaxLevel MaxLevelView::getMaxLevel() const
{
  return fState->getMaxLevel(fLCDInputXParameter->getValue());
}


MaxLevelView::Creator __gMaxLevelViewCreator("pongasoft::MaxLevel", "pongasoft - Max Level");
}
}
}