#pragma once

#include <vstgui4/vstgui/lib/controls/ccontrol.h>

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

// TODO TODO TODO
/* at this time I am still getting unexplained behavior (if the mouse click/release is too fast)
2018-06-11 10:47:59.350 ( 177.197s) [           1F4B2]    MomentaryButton.cpp:60       0| MomentaryButton::onMouseDown
2018-06-11 10:47:59.356 ( 177.204s) [           1F561]      VAC6Processor.cpp:263      0| VAC6Processor::processParameters => kMaxLevelReset=1.000000
2018-06-11 10:47:59.397 ( 177.244s) [           1F4B2]    MomentaryButton.cpp:78       0| MomentaryButton::onMouseUp
2018-06-11 10:47:59.399 ( 177.246s) [           1F561]      VAC6Processor.cpp:263      0| VAC6Processor::processParameters => kMaxLevelReset=0.000000
2018-06-11 10:47:59.432 ( 177.280s) [           1F4B2]      VAC6Processor.cpp:307      0| VAC6Processor::getState => fSoftClippingLevel=0.170968
2018-06-11 10:47:59.432 ( 177.280s) [           1F4B2]     VAC6Controller.cpp:168      0| VAC6Controller::getState()
2018-06-11 10:47:59.433 ( 177.281s) [           1F4B2]      VAC6Processor.cpp:307      0| VAC6Processor::getState => fSoftClippingLevel=0.170968
2018-06-11 10:47:59.433 ( 177.281s) [           1F4B2]     VAC6Controller.cpp:168      0| VAC6Controller::getState()
2018-06-11 10:47:59.441 ( 177.289s) [           1F561]      VAC6Processor.cpp:263      0| VAC6Processor::processParameters => kMaxLevelReset=1.000000
 */

/**
 * Represents a momentary button: a button which is "on" only when pressed. The VSTGUI sdk has a class called
 * CKickButton which is similar but it behaves improperly (for example, the button gets stuck in "pressed" state
 * for some reason...)
 */
class MomentaryButton : public CControl
{
public:
  MomentaryButton(const CRect &size, IControlListener *listener, int32_t tag, CBitmap *pBackground);

  MomentaryButton(const MomentaryButton &momentaryButton);

  void draw(CDrawContext *) override;

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;

  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override;

  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override;

  CMouseEventResult onMouseCancel() override;

  int32_t onKeyDown(VstKeyCode &keyCode) override;

  int32_t onKeyUp(VstKeyCode &keyCode) override;

  bool sizeToFit() override;

  CLASS_METHODS(MomentaryButton, CControl)

protected:
  ~MomentaryButton() noexcept override = default;

  void updateValue(float fNewValue);
};

}
}
}
