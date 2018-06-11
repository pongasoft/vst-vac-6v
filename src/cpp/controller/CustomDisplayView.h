#pragma once

#include <vstgui4/vstgui/lib/controls/ccontrol.h>

#include <utility>

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

}
}
}

