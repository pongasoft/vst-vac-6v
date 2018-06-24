#include <vstgui4/vstgui/uidescription/iviewcreator.h>
#include <vstgui4/vstgui/uidescription/uiviewcreator.h>
#include <vstgui4/vstgui/uidescription/uiattributes.h>
#include <vstgui4/vstgui/uidescription/detail/uiviewcreatorattributes.h>
#include <vstgui4/vstgui/lib/cdrawcontext.h>
#include "CustomDisplayView.h"
#include "../logging/loguru.hpp"
#include "CustomView.h"


namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

///////////////////////////////////////////
// CustomDisplayView::CustomDisplayView
///////////////////////////////////////////
CustomDisplayView::CustomDisplayView(const CRect &size, IControlListener *listener, int32_t tag, CBitmap *pBackground)
  : CControl(size, listener, tag, pBackground),
    fDrawCallback{nullptr},
    fBackColor{0,0,0}
{
  DLOG_F(INFO, "CustomDisplayView::CustomDisplayView()");
  setWantsFocus(true);
}

///////////////////////////////////////////
// CustomDisplayView::CustomDisplayView
///////////////////////////////////////////
CustomDisplayView::CustomDisplayView(const CustomDisplayView &c) : CControl(c)
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

///////////////////////////////////////////
// CustomDisplayCreator
///////////////////////////////////////////

class CustomDisplayCreator : public CustomViewCreator<CustomDisplayView>
{
public:
  explicit CustomDisplayCreator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) : CustomViewCreator(iViewName, iDisplayName)
  {
    registerColorAttribute(UIViewCreator::kAttrBackColor, &CustomDisplayView::getBackColor, &CustomDisplayView::setBackColor);
  }
};


CustomDisplayCreator __gLCDDisplayCreator2("pongasoft::CustomDisplay", "pongasoft - Custom Display");

}
}
}

