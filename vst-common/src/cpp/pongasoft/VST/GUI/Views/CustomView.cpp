#include <vstgui4/vstgui/uidescription/iviewcreator.h>
#include <vstgui4/vstgui/lib/cdrawcontext.h>
#include "CustomView.h"
#include "pongasoft/VST/GUI/Views/CustomViewFactory.h"

namespace pongasoft {
namespace VST {
namespace GUI {
namespace Views {

using namespace VSTGUI;

///////////////////////////////////////////
// CustomView::CustomView
///////////////////////////////////////////
CustomView::CustomView(const CRect &iSize)
  : CView(iSize),
    fTag{-1},
    fBackColor{0, 0, 0},
    fEditorMode{false},
    fParamCxMgr{nullptr}
{
  setWantsFocus(true);
}

///////////////////////////////////////////
// CustomView::draw
///////////////////////////////////////////
void CustomView::draw(CDrawContext *iContext)
{
  if(getBackColor().alpha != 0)
  {
    iContext->setFillColor(getBackColor());
    iContext->drawRect(getViewSize(), kDrawFilled);
  }

  setDirty(false);
}

///////////////////////////////////////////
// CustomView::setBackColor
///////////////////////////////////////////
void CustomView::setBackColor(CColor const &iColor)
{
  // to force the redraw
  if(fBackColor != iColor)
  {
    fBackColor = iColor;
    drawStyleChanged();
  }
}

///////////////////////////////////////////
// CustomView::drawStyleChanged
///////////////////////////////////////////
void CustomView::drawStyleChanged()
{
  setDirty(true);
}

///////////////////////////////////////////
// CustomView::onParameterChange
///////////////////////////////////////////
void CustomView::onParameterChange(ParamID iParamID, ParamValue iNormalizedValue)
{
  // DLOG_F(INFO, "CustomView::onParameterChange(%d, %d, %f)", fTag, iParamID, iNormalizedValue);
  setDirty(true);
}

///////////////////////////////////////////
// CustomView::registerGUIRawParam
///////////////////////////////////////////
std::unique_ptr<GUIRawParameter> CustomView::registerGUIRawParam(ParamID iParamID, bool iSubscribeToChanges)
{
  if(!fParamCxMgr)
    ABORT_F("fParamCxMgr should have been registered");

  return fParamCxMgr->registerGUIRawParam(iParamID, iSubscribeToChanges ? this : nullptr);
}

///////////////////////////////////////////
// CustomView::registerBooleanParam
///////////////////////////////////////////
GUIParamUPtr<BooleanParamConverter> CustomView::registerBooleanParam(ParamID iParamID, bool iSubscribeToChanges)
{
  return registerGUIParam<BooleanParamConverter>(iParamID, iSubscribeToChanges);
}

///////////////////////////////////////////
// CustomView::registerPercentParam
///////////////////////////////////////////
GUIParamUPtr<PercentParamConverter> CustomView::registerPercentParam(ParamID iParamID, bool iSubscribeToChanges)
{
  return registerGUIParam<PercentParamConverter>(iParamID, iSubscribeToChanges);
}

///////////////////////////////////////////
// CustomView::setEditorMode
///////////////////////////////////////////
void CustomView::setEditorMode(bool iEditorMode)
{
#if EDITOR_MODE
  if(fEditorMode != iEditorMode)
  {
    fEditorMode = iEditorMode;
    onEditorModeChanged();
  }
#endif

  // when not in editor mode, this does nothing...
}

///////////////////////////////////////////
// CustomView::getEditorMode
///////////////////////////////////////////
bool CustomView::getEditorMode() const
{
#if EDITOR_MODE
  return fEditorMode;
#else
  return false;
#endif
}

///////////////////////////////////////////
// CustomView::afterCreate
///////////////////////////////////////////
void CustomView::afterCreate(UIAttributes const &iAttributes, IUIDescription const *iDescription)
{
  auto provider = dynamic_cast<GUIParametersProvider const *>(iDescription->getViewFactory());
  if(provider)
    initParameters(provider->getGUIParameters());
}

///////////////////////////////////////////
// CustomView::beforeApply
///////////////////////////////////////////
void CustomView::beforeApply(UIAttributes const &iAttributes, IUIDescription const *iDescription)
{
  // nothing to do...
}

///////////////////////////////////////////
// CustomView::afterApply
///////////////////////////////////////////
void CustomView::afterApply(UIAttributes const &iAttributes, IUIDescription const *iDescription)
{
  if(fParamCxMgr)
    registerParameters();
}

}
}
}
}