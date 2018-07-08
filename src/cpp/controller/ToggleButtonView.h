#pragma once

#include "CustomView.h"

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

class ToggleButtonView : public TCustomControlView<BooleanParameter>
{
public:
  explicit ToggleButtonView(const CRect &iSize) : TCustomControlView(iSize) {}

  // draw => does the actual drawing job
  void draw(CDrawContext *iContext) override;

  // onMouseDown
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;

  // onMouseUp
  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override;

  // onMouseCancel
  CMouseEventResult onMouseCancel() override;

  // is on or off
  bool isOn() const { return getControlValue(); }
  bool isOff() const { return !isOn(); }

public:
  CLASS_METHODS_NOCOPY(ToggleButtonView, CustomControlView)

protected:
  bool fPressed{false};

public:
  class Creator : public CustomViewCreator<ToggleButtonView>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerAttributes(TCustomControlView<BooleanParameter>::Creator());
    }
  };
};

}
}
}