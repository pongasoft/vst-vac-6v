#include <vstgui4/vstgui/uidescription/iviewcreator.h>
#include <vstgui4/vstgui/lib/cdrawcontext.h>
#include "CustomDisplayView.h"


namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

///////////////////////////////////////////
// CustomDisplayView::CustomDisplayView
///////////////////////////////////////////
CustomDisplayView::CustomDisplayView(const CRect &size)
  : CView(size),
    fDrawCallback{nullptr},
    fBackColor{0,0,0}
{
  DLOG_F(INFO, "CustomDisplayView::CustomDisplayView()");
  setWantsFocus(true);
}

///////////////////////////////////////////
// CustomDisplayView::CustomDisplayView
///////////////////////////////////////////
CustomDisplayView::CustomDisplayView(const CustomDisplayView &c) :
  CView(c),
  fDrawCallback{c.fDrawCallback},
  fBackColor{c.fBackColor}
{
  DLOG_F(INFO, "CustomDisplayView::CustomDisplayView(const CustomDisplayView &c)");
  setWantsFocus(true);
}

///////////////////////////////////////////
// CustomDisplayView::draw
///////////////////////////////////////////
void CustomDisplayView::draw(CDrawContext *iContext)
{
  iContext->setFillColor(getBackColor());
  iContext->drawRect(getViewSize(), kDrawFilled);

  if(fDrawCallback != nullptr)
    fDrawCallback(this, iContext);

  setDirty(false);
}

///////////////////////////////////////////
// CustomDisplayView::setBackColor
///////////////////////////////////////////
void CustomDisplayView::setBackColor(CColor const &color)
{
  // to force the redraw
  if(fBackColor != color)
  {
    fBackColor = color;
    drawStyleChanged();
  }
}

///////////////////////////////////////////
// CustomDisplayView::drawStyleChanged
///////////////////////////////////////////
void CustomDisplayView::drawStyleChanged()
{
  setDirty(true);
}

CustomDisplayCreator __gCustomDisplayCreator("pongasoft::CustomDisplay", "pongasoft - Custom Display");
}
}
}

