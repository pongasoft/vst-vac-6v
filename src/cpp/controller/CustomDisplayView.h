#pragma once

#include <vstgui4/vstgui/lib/controls/ccontrol.h>
#include "CustomView.h"

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

class CustomDisplayView : public CControl
{
  using CustomDisplayDrawCallback = std::function<void(CustomDisplayView *, CDrawContext *)>;

public:
  CustomDisplayView(const CRect &size, IControlListener *listener, int32_t tag, CBitmap *pBackground);

  CustomDisplayView(const CustomDisplayView &c);

  void setBackColor(CColor const &color);
  CColor const &getBackColor() const { return fBackColor; }

  void setDrawCallback(CustomDisplayDrawCallback iDrawCallback) { fDrawCallback = std::move(iDrawCallback); }
  CustomDisplayDrawCallback getDrawCallback() const { return fDrawCallback; }

  void draw(CDrawContext *iContext) override;

  void drawStyleChanged();

  CLASS_METHODS(CustomDisplayView, CControl)

protected:
  CustomDisplayDrawCallback fDrawCallback;

protected:
  CColor fBackColor;
};

///////////////////////////////////////////
// CustomDisplayCreator
///////////////////////////////////////////

class CustomDisplayCreator : public CustomViewCreator<CustomDisplayView>
{
public:
  explicit CustomDisplayCreator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
    CustomViewCreator(iViewName, iDisplayName)
  {
    registerColorAttribute(UIViewCreator::kAttrBackColor, &CustomDisplayView::getBackColor, &CustomDisplayView::setBackColor);
  }
};


}
}
}

