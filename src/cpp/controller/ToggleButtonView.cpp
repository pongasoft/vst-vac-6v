#include "ToggleButtonView.h"

#include <vstgui4/vstgui/lib/cdrawcontext.h>

namespace pongasoft {
namespace VST {
namespace GUI {

///////////////////////////////////////////
// ToggleButtonView::setFrames
///////////////////////////////////////////
void ToggleButtonView::setFrames(int iFrames)
{
  if(iFrames != 2 && iFrames != 4)
  {
    DLOG_F(WARNING, "frames should be either 2 or 4");
  }
  else
    fFrames = iFrames;
}

///////////////////////////////////////////
// ToggleButtonView::draw
///////////////////////////////////////////
void ToggleButtonView::draw(CDrawContext *iContext)
{
  CustomView::draw(iContext);

  if(fImage)
  {
    int frameIndex;
    CCoord frameHeight = fImage->getHeight() / getFrames();
    if(getFrames() == 4)
    {
      frameIndex = isOn() ? 2 : 0;
      if(fPressed)
        frameIndex++;
    }
    else
    {
      frameIndex = isOn() ? 1 : 0;
    }

    fImage->draw(iContext, getViewSize(), CPoint{0, frameIndex * frameHeight});
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

    if(getFrames() == 4)
    {
      if(fPressed)
      {
        iContext->setFillColor(CColor{0,0,0,120});
        iContext->drawRect(getViewSize(), kDrawFilled);
      }
    }
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
