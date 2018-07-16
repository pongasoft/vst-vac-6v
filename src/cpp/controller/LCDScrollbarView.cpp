#include "LCDScrollbarView.h"
#include "../VAC6CIDs.h"

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
  fLCDZoomFactorXParameter = registerVSTParameter<LCDZoomFactorXParameter>(EVAC6ParamID::kLCDZoomFactorX);
}

///////////////////////////////////////////
// LCDScrollbarView::computeZoomBox
LCDScrollbarView::ZoomBox LCDScrollbarView::computeZoomBox() const
{
  ZoomBox box{};

  CCoord width = getViewSize().getWidth();

  box.fHalfWidth = (width - ((width - getScrollbarMinSize()) * fLCDZoomFactorXParameter->getValue())) / 2.0;
  box.fMinCenter = 0 + box.fHalfWidth;
  box.fMaxCenter = width - box.fHalfWidth;
  box.fCenter = box.computeCenter(fLCDInputHistoryOffsetParameter->getValue());

  return box;
}

///////////////////////////////////////////
// LCDScrollbarView::draw
///////////////////////////////////////////
void LCDScrollbarView::draw(CDrawContext *iContext)
{
  CustomView::draw(iContext);

  auto rdc = GUI::RelativeDrawContext{this, iContext};

  auto box = computeZoomBox();

  auto verticalMargin = 3.5;
  auto horizontalMargin = 2.5;

  rdc.fillRect(RelativeRect{box.getLeft() + horizontalMargin,
                            verticalMargin,
                            box.getRight() - horizontalMargin,
                            getViewSize().getHeight() - verticalMargin},
               getHandleColor());
}

///////////////////////////////////////////
// LCDScrollbarView::onMouseDown
///////////////////////////////////////////
CMouseEventResult LCDScrollbarView::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  RelativeView rv(this);
  RelativeCoord x = rv.fromAbsolutePoint(where).x;

  if(fLCDLiveViewParameter->getValue())
  {
    fLCDLiveViewParameter->setValue(false);
  }

  auto box = computeZoomBox();

  // when the box is completely full we can't really act on it...
  if(box.isFull())
    return kMouseEventHandled;

  if(x < box.getLeft())
  {
    box.move(-box.getWidth());
  }
  else
  {
    if(x > box.getRight())
    {
      box.move(box.getWidth());
    }
    else
    {
      // beginning of drag gesture...
      fLCDInputHistoryOffsetEditor = fLCDInputHistoryOffsetParameter->edit(box.computePercent());
      fStartDragGestureZoomBox = std::make_unique<ZoomBox>(box);
      fStarDragGestureX = x;
      return kMouseEventHandled;
    }
  }

  // if after move, the cursor is in the handle => allow for drag
  if(x >= box.getLeft() && x <= box.getRight())
  {
    // beginning of drag gesture...
    fLCDInputHistoryOffsetEditor = fLCDInputHistoryOffsetParameter->edit(box.computePercent());
    fStartDragGestureZoomBox = std::make_unique<ZoomBox>(box);
    fStarDragGestureX = x;
    return kMouseEventHandled;
  }

  // no drag gesture
  fLCDInputHistoryOffsetParameter->setValue(box.computePercent());

  return kMouseEventHandled;
}

///////////////////////////////////////////
// LCDScrollbarView::onMouseMoved
///////////////////////////////////////////
CMouseEventResult LCDScrollbarView::onMouseMoved(CPoint &where, const CButtonState &buttons)
{
  if(fLCDInputHistoryOffsetEditor)
  {
    RelativeView rv(this);
    RelativeCoord x = rv.fromAbsolutePoint(where).x;

    auto box = ZoomBox(*fStartDragGestureZoomBox);
    box.move(x - fStarDragGestureX);
    fLCDInputHistoryOffsetEditor->setValue(box.computePercent());

    return kMouseEventHandled;
  }

  return kMouseEventNotHandled;
}

///////////////////////////////////////////
// LCDScrollbarView::onMouseUp
///////////////////////////////////////////
CMouseEventResult LCDScrollbarView::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  if(fLCDInputHistoryOffsetEditor)
  {
    fLCDInputHistoryOffsetEditor->commit();
    fLCDInputHistoryOffsetEditor = nullptr;
    fStartDragGestureZoomBox = nullptr;
    fStarDragGestureX = -1;
    return kMouseEventHandled;
  }

  return kMouseEventNotHandled;
}

///////////////////////////////////////////
// LCDScrollbarView::onMouseCancel
///////////////////////////////////////////
CMouseEventResult LCDScrollbarView::onMouseCancel()
{
  if(fLCDInputHistoryOffsetEditor)
  {
    fLCDInputHistoryOffsetEditor->rollback();
    fLCDInputHistoryOffsetEditor = nullptr;
    fStartDragGestureZoomBox = nullptr;
    fStarDragGestureX = -1;
    return kMouseEventHandled;
  }

  return kMouseEventNotHandled;
}

LCDScrollbarView::Creator __gLCDScrollbarViewCreator("pongasoft::LCDScrollbar", "pongasoft - LCD Scrollbar");

}
}
}