#include "ToggleButtonView.h"

#include <vstgui4/vstgui/lib/cdrawcontext.h>

namespace pongasoft {
namespace VST {
namespace GUI {

///////////////////////////////////////////
// ToggleButtonView::draw
///////////////////////////////////////////
void ToggleButtonView::draw(CDrawContext *iContext)
{
  CustomView::draw(iContext);

  if(isOn())
  {
    iContext->setFillColor(kRedCColor);
    iContext->drawRect(getViewSize(), kDrawFilled);
  }

  if(fPressed)
  {
    iContext->setFillColor(CColor{0,0,0,120});
    iContext->drawRect(getViewSize(), kDrawFilled);
  }
}

///////////////////////////////////////////
// ToggleButtonView::onMouseDown
///////////////////////////////////////////
CMouseEventResult ToggleButtonView::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  fPressed = true;
  setDirty(true);
  return kMouseEventHandled;
}

///////////////////////////////////////////
// ToggleButtonView::onMouseUp
///////////////////////////////////////////
CMouseEventResult ToggleButtonView::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  fPressed = false;
  setControlValue(!getControlValue());
  setDirty(true);
  return kMouseEventHandled;
}

///////////////////////////////////////////
// ToggleButtonView::onMouseCancel
///////////////////////////////////////////
CMouseEventResult ToggleButtonView::onMouseCancel()
{
  fPressed = false;
  setDirty(true);
  return kMouseEventHandled;
}

ToggleButtonView::Creator __gToggleButtonCreator("pongasoft::ToggleButton", "pongasoft - Toggle Button (on/off)");
}
}
}
