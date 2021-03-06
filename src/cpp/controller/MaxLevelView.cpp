#include <pongasoft/VST/GUI/DrawContext.h>
#include "MaxLevelView.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

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

  auto maxLevel = getMaxLevel();

  CColor fontColor = maxLevel.isUndefined() ?
                     getNoDataColor() :
                     computeColor(*fSoftClippingLevelParameter, maxLevel.fValue);

  auto rdc = GUI::RelativeDrawContext{this, iContext};

  StringDrawContext sdc{};
  sdc.fHorizTxtAlign = kCenterText;
  sdc.fTextInset = {2, 2};
  sdc.fFontColor = fontColor;
  sdc.fFont = fFont;

  rdc.drawString(maxLevel.toDbString(), sdc);
}

///////////////////////////////////////////
// MaxLevelView::getMaxLevel
///////////////////////////////////////////
MaxLevel MaxLevelView::getMaxLevel() const
{
  switch(fType)
  {
    case Type::kForSelection:
      return fHistoryDataParam->getMaxLevelForSelection(*fLCDInputXParameter);

    case Type::kSinceReset:
      return fHistoryDataParam->fMaxLevelSinceReset;

    case Type::kInWindow:
      return fHistoryDataParam->fMaxLevelInWindow;

    default:
      DLOG_F(WARNING, "should not be reached");
      return MaxLevel{};
  }
}


MaxLevelView::Creator __gMaxLevelViewCreator("VAC6V::MaxLevel", "VAC6V - Max Level");
}
}
}