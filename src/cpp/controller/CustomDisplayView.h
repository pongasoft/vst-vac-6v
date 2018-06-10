#pragma once

#include <vstgui4/vstgui/lib/controls/ccontrol.h>

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

class CustomDisplayView : public CControl
{
public:
  CustomDisplayView(const CRect &size, IControlListener *listener, int32_t tag, CBitmap *pBackground);

  CustomDisplayView(const CustomDisplayView &c);

  virtual void setBackColor(CColor const &color);

  CColor const &getBackColor() const { return fBackColor; }

  void draw(CDrawContext *pContext) override;

  virtual void drawStyleChanged();

  CLASS_METHODS(CustomDisplayView, CControl)

protected:
  CColor fBackColor;
};

}
}
}

