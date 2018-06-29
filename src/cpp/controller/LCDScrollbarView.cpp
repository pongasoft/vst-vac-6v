#include "LCDScrollbarView.h"
#include "../VAC6CIDs.h"
#include "../Utils.h"
#include "DrawContext.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// LCDScrollbarView::LCDScrollbarView
///////////////////////////////////////////
LCDScrollbarView::LCDScrollbarView(const CRect &size) : CustomView(size)
{
}

///////////////////////////////////////////
// LCDScrollbarView::registerParameters
///////////////////////////////////////////
void LCDScrollbarView::registerParameters()
{
  fLCDLiveViewParameter = registerBooleanParameter(EVAC6ParamID::kLCDLiveView);
  fLCDInputHistoryOffsetParameter = registerPercentParameter(EVAC6ParamID::kLCDHistoryOffset);
}

///////////////////////////////////////////
// LCDScrollbarView::draw
///////////////////////////////////////////
void LCDScrollbarView::draw(CDrawContext *iContext)
{
  CustomView::draw(iContext);

  // in live view mode we don't show anything...
  if(fLCDLiveViewParameter->getValue())
    return;

  auto rdc = GUI::RelativeDrawContext{this, iContext};

  auto height = getViewSize().getHeight();
  auto width = getViewSize().getWidth();

  ParamValue inputHistoryOffsetPercent = fLCDInputHistoryOffsetParameter->getValue();
  CCoord inputHistoryX = Utils::Lerp<double>(0, width).compute(inputHistoryOffsetPercent);

  DLOG_F(INFO, "LCDScrollbarView::draw -> %f / %f ", inputHistoryOffsetPercent, inputHistoryX);

  CCoord inputHistoryY = height / 2.0;

  rdc.fillRect(RelativeRect{inputHistoryX - 3, inputHistoryY - 3, inputHistoryX + 3, inputHistoryY + 3}, WHITE_COLOR);
}

///////////////////////////////////////////
// LCDScrollbarView::draw
///////////////////////////////////////////
CMouseEventResult LCDScrollbarView::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  RelativeView rv(this);
  RelativePoint relativeWhere = rv.fromAbsolutePoint(where);

  DLOG_F(INFO, "LCDScrollbarView::onMouseDown(%f,%f)", relativeWhere.x, relativeWhere.y);

  if(fLCDLiveViewParameter->getValue())
  {
    DLOG_F(INFO, "LCDScrollbarView::onMouseDown() => stopping");
    fLCDLiveViewParameter->setValue(false);
  }

  DLOG_F(INFO, "LCDScrollbarView -> %f", Utils::Lerp<double>(0, getViewSize().getWidth()).reverse(relativeWhere.x));

  fLCDInputHistoryOffsetParameter->setValue(Utils::Lerp<double>(0, getViewSize().getWidth()).reverse(relativeWhere.x));

  return kMouseEventHandled;
}

LCDScrollbarView::Creator __gLCDScrollbarViewCreator("pongasoft::LCDScrollbar", "pongasoft - LCD Scrollbar");

}
}
}