#include "MomentaryButton.h"

#include <vstgui4/vstgui/lib/cdrawcontext.h>

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

///////////////////////////////////////////
// MomentaryButton::draw
///////////////////////////////////////////
void MomentaryButton::draw(CDrawContext *iContext)
{
  CustomView::draw(iContext);

  if(fImage)
  {
    CCoord y = isOn() ? fImage->getHeight() / 2 : 0;
    fImage->draw(iContext, getViewSize(), CPoint{0, y});
  }
  else
  {
    // no image => simply fill the surface with appropriate color (background and "on" color)
    // so that the button is fully functioning right away
    if(isOn())
    {
      iContext->setFillColor(getOnColor());
      iContext->drawRect(getViewSize(), kDrawFilled);
    }
  }

  setDirty(false);
}

///////////////////////////////////////////
// MomentaryButton::onMouseDown
///////////////////////////////////////////
CMouseEventResult MomentaryButton::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  if(!(buttons & kLButton))
    return kMouseEventNotHandled;

  setControlValue(true);
  setDirty(true);
  return kMouseEventHandled;
}

///////////////////////////////////////////
// MomentaryButton::onMouseUp
///////////////////////////////////////////
CMouseEventResult MomentaryButton::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  if(!(buttons & kLButton))
    return kMouseEventNotHandled;

  setControlValue(false);
  setDirty(true);
  return kMouseEventHandled;
}

///////////////////////////////////////////
// MomentaryButton::onMouseCancel
///////////////////////////////////////////
CMouseEventResult MomentaryButton::onMouseCancel()
{
  setControlValue(false);
  setDirty(true);
  return kMouseEventHandled;
}

///////////////////////////////////////////
// MomentaryButton::onKeyDown
///////////////////////////////////////////
int32_t MomentaryButton::onKeyDown(VstKeyCode &keyCode)
{
  if(keyCode.virt == VKEY_RETURN && keyCode.modifier == 0)
  {
    setControlValue(true);
    setDirty(true);
    return 1;
  }
  return -1;
}

///////////////////////////////////////////
// MomentaryButton::onKeyUp
///////////////////////////////////////////
int32_t MomentaryButton::onKeyUp(VstKeyCode &keyCode)
{
  if(keyCode.virt == VKEY_RETURN && keyCode.modifier == 0)
  {
    setControlValue(false);
    setDirty(true);
    return 1;
  }
  return -1;
}

///////////////////////////////////////////
// MomentaryButton::sizeToFit
///////////////////////////////////////////
bool MomentaryButton::sizeToFit()
{
  DLOG_F(INFO, "MomentaryButton::sizeToFit");
  return CustomView::sizeToFit(getImage(), 2);
}

MomentaryButton::Creator __gMomentaryButtonCreator("pongasoft::MomentaryButton", "pongasoft - Momentary Button (on when pressed)");

}
}
}