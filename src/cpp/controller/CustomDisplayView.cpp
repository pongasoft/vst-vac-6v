#include <vstgui4/vstgui/uidescription/iviewcreator.h>
#include <vstgui4/vstgui/uidescription/uiviewcreator.h>
#include <vstgui4/vstgui/uidescription/uiattributes.h>
#include <vstgui4/vstgui/uidescription/uiviewfactory.h>
#include <vstgui4/vstgui/uidescription/detail/uiviewcreatorattributes.h>
#include <vstgui4/vstgui/lib/cdrawcontext.h>
#include "CustomDisplayView.h"
#include "../logging/loguru.hpp"


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

class CustomDisplayCreator : public ViewCreatorAdapter
{
public:
  static constexpr IdStringPtr kCustomDisplay = "CustomDisplay";

  CustomDisplayCreator()
  {
    //DLOG_F(INFO, "CustomDisplayView::CustomDisplayView(const CustomDisplayView &c)");
    VSTGUI::UIViewFactory::registerViewCreator(*this);
  }

  IdStringPtr getViewName() const override
  {
    return kCustomDisplay;
  }

  IdStringPtr getBaseViewName() const override
  {
    return VSTGUI::UIViewCreator::kCControl;
  }

  UTF8StringPtr getDisplayName() const override
  {
    return "pongasoft - Custom Display";
  }

  CView *create(const UIAttributes &attributes, const IUIDescription *description) const override
  {
    DLOG_F(INFO, "CustomDisplayCreator::create()");
    return new CustomDisplayView(CRect(0, 0, 0, 0), nullptr, -1, nullptr);
  }

  bool apply(CView *view, const UIAttributes &attributes, const IUIDescription *description) const override
  {
    auto *cdv = dynamic_cast<CustomDisplayView *>(view);

    if(cdv == nullptr)
      return false;

    CColor color;

    if(UIViewCreator::stringToColor(attributes.getAttributeValue(UIViewCreator::kAttrBackColor), color, description))
      cdv->setBackColor(color);


    return true;
  }

  bool getAttributeNames(std::list<std::string> &attributeNames) const override
  {
    attributeNames.emplace_back(UIViewCreator::kAttrBackColor);
    return true;
  }

  AttrType getAttributeType(const std::string &attributeName) const override
  {
    if(attributeName == UIViewCreator::kAttrBackColor) return kColorType;
    return kUnknownType;
  }

  bool getAttributeValue(CView *view, const std::string &attributeName, std::string &stringValue,
                         const IUIDescription *desc) const override
  {
    auto *cdv = dynamic_cast<CustomDisplayView *>(view);

    if(cdv == nullptr)
      return false;

    else if(attributeName == UIViewCreator::kAttrBackColor)
    {
      UIViewCreator::colorToString(cdv->getBackColor(), stringValue, desc);
      return true;
    }

    return false;
  }
};

CustomDisplayCreator __gLCDDisplayCreator;

}
}
}

