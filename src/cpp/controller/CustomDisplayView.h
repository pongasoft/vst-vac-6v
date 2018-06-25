#pragma once

#include <vstgui4/vstgui/lib/cview.h>
#include "CustomView.h"

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

class CustomDisplayView : public CView
{
  using CustomDisplayDrawCallback = std::function<void(CustomDisplayView *, CDrawContext *)>;

public:
  explicit CustomDisplayView(const CRect &size);

  CustomDisplayView(const CustomDisplayView &c);

  void setBackColor(CColor const &color);
  CColor const &getBackColor() const { return fBackColor; }

  void setDrawCallback(CustomDisplayDrawCallback iDrawCallback) { fDrawCallback = std::move(iDrawCallback); }
  CustomDisplayDrawCallback getDrawCallback() const { return fDrawCallback; }

  void setCustomViewTag (int32_t iTag) { fTag = iTag; }
  int32_t getCustomViewTag () const { return fTag; }

  void draw(CDrawContext *iContext) override;

  void drawStyleChanged();

  CLASS_METHODS(CustomDisplayView, CControl)

protected:
  CustomDisplayDrawCallback fDrawCallback;

protected:
  int32_t fTag;
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
    registerTagAttribute("custom-display-view-tag", &CustomDisplayView::getCustomViewTag, &CustomDisplayView::setCustomViewTag);
    registerColorAttribute(UIViewCreator::kAttrBackColor, &CustomDisplayView::getBackColor, &CustomDisplayView::setBackColor);
  }
};


}
}
}

