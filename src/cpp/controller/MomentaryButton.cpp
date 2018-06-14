#include "MomentaryButton.h"
#include "../logging/loguru.hpp"
#include <vstgui4/vstgui/lib/cbitmap.h>
#include <vstgui4/vstgui/uidescription/uiviewfactory.h>
#include <vstgui4/vstgui/uidescription/detail/uiviewcreatorattributes.h>

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

///////////////////////////////////////////
// MomentaryButton::MomentaryButton
///////////////////////////////////////////
MomentaryButton::MomentaryButton(const CRect &size, IControlListener *listener, int32_t tag, CBitmap *pBackground)
  : CControl(size, listener, tag, pBackground)
{

}

///////////////////////////////////////////
// MomentaryButton::MomentaryButton
///////////////////////////////////////////
MomentaryButton::MomentaryButton(const MomentaryButton &momentaryButton)
  : CControl(momentaryButton)
{
  setWantsFocus(true);
}

///////////////////////////////////////////
// MomentaryButton::draw
///////////////////////////////////////////
void MomentaryButton::draw(CDrawContext *fContext)
{
//  DLOG_F(INFO, "MomentaryButton::draw(%f)", value);

  CPoint where(0, 0);

  CBitmap *background = getDrawBackground();

  if(background)
  {
    if(value == getMax())
      where.y = background->getHeight() / 2.0;

    background->draw(fContext, getViewSize(), where);
  }

  setDirty(false);
}

///////////////////////////////////////////
// MomentaryButton::updateValue
///////////////////////////////////////////
void MomentaryButton::updateValue(float fNewValue)
{
  if(value != fNewValue)
  {
    beginEdit();

    value = fNewValue;

    DLOG_F(INFO, "MomentaryButton::updateValue(%f)", value);

    invalid();
    valueChanged();

    endEdit();
  }
}

///////////////////////////////////////////
// MomentaryButton::onMouseDown
///////////////////////////////////////////
CMouseEventResult MomentaryButton::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  DLOG_F(INFO, "MomentaryButton::onMouseDown");

  if(!(buttons & kLButton))
    return kMouseEventNotHandled;

  updateValue(getMax());

  return kMouseEventHandled;
}

///////////////////////////////////////////
// MomentaryButton::onMouseMoved
///////////////////////////////////////////
CMouseEventResult MomentaryButton::onMouseMoved(CPoint &where, const CButtonState &buttons)
{
  // nothing to do when the mouse moves...
  return kMouseEventHandled;
}

///////////////////////////////////////////
// MomentaryButton::onMouseUp
///////////////////////////////////////////
CMouseEventResult MomentaryButton::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  DLOG_F(INFO, "MomentaryButton::onMouseUp");
  updateValue(getMin());
  return kMouseEventHandled;
}

///////////////////////////////////////////
// MomentaryButton::onMouseCancel
///////////////////////////////////////////
CMouseEventResult MomentaryButton::onMouseCancel()
{
  DLOG_F(INFO, "MomentaryButton::onMouseCancel");
  updateValue(getMin());
  return kMouseEventHandled;
}

///////////////////////////////////////////
// MomentaryButton::onKeyDown
///////////////////////////////////////////
int32_t MomentaryButton::onKeyDown(VstKeyCode &keyCode)
{
  DLOG_F(INFO, "MomentaryButton::onKeyDown");
  if(keyCode.virt == VKEY_RETURN && keyCode.modifier == 0)
  {
    updateValue(getMax());
    return 1;
  }
  return -1;
}

///////////////////////////////////////////
// MomentaryButton::onKeyUp
///////////////////////////////////////////
int32_t MomentaryButton::onKeyUp(VstKeyCode &keyCode)
{
  DLOG_F(INFO, "MomentaryButton::onKeyUp");
  if(keyCode.virt == VKEY_RETURN && keyCode.modifier == 0)
  {
    updateValue(getMin());
    return 1;
  }
  return -1;
}

///////////////////////////////////////////
// MomentaryButton::sizeToFit
///////////////////////////////////////////
bool MomentaryButton::sizeToFit()
{
  CBitmap *background = getDrawBackground();
  if(background)
  {
    CRect vs(getViewSize());
    vs.setWidth(background->getWidth());
    vs.setHeight(background->getHeight() / 2.0);
    setViewSize(vs, true);
    setMouseableArea(vs);
    return true;
  }
  return false;
}

///////////////////////////////////////////
// MomentaryButtonCreator
///////////////////////////////////////////
class MomentaryButtonCreator : public ViewCreatorAdapter
{
public:
  static constexpr IdStringPtr kMomentaryButton = "pongasoft::MomentaryButton";

  MomentaryButtonCreator()
  {
    UIViewFactory::registerViewCreator(*this);
  }

  IdStringPtr getViewName() const override
  {
    return kMomentaryButton;
  }

  IdStringPtr getBaseViewName() const override
  {
    return VSTGUI::UIViewCreator::kCControl;
  }

  UTF8StringPtr getDisplayName() const override
  {
    return "pongasoft - Momentary Button";
  }

  CView *create(const UIAttributes &attributes, const IUIDescription *description) const override
  { return new MomentaryButton(CRect(0, 0, 0, 0), nullptr, -1, nullptr); }
};

MomentaryButtonCreator __gMomentaryButtonCreator;

}
}
}