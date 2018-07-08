#include <vstgui4/vstgui/uidescription/iviewcreator.h>
#include <vstgui4/vstgui/lib/cdrawcontext.h>
#include "CustomView.h"
#include "CustomViewFactory.h"


namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

///////////////////////////////////////////
// CustomView::CustomView
///////////////////////////////////////////
CustomView::CustomView(const CRect &iSize)
  : CView(iSize),
    fTag{-1},
    fBackColor{0,0,0},
    fEditorMode{false},
    fParameters{nullptr}
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
// CustomView::registerRawParameter
///////////////////////////////////////////
std::unique_ptr<RawParameter> CustomView::registerRawParameter(ParamID iParamID, bool iSubscribeToChanges)
{
  if(!fParameters)
    ABORT_F("fParameters should have been registered");

  return fParameters->registerRawParameter(iParamID, iSubscribeToChanges ? this : nullptr);
}

///////////////////////////////////////////
// CustomView::registerBooleanParameter
///////////////////////////////////////////
std::unique_ptr<BooleanParameter> CustomView::registerBooleanParameter(ParamID iParamID, bool iSubscribeToChanges)
{
  return registerVSTParameter<BooleanParameter>(iParamID, iSubscribeToChanges);
}

///////////////////////////////////////////
// CustomView::registerPercentParameter
///////////////////////////////////////////
std::unique_ptr<PercentParameter> CustomView::registerPercentParameter(ParamID iParamID, bool iSubscribeToChanges)
{
  return registerRawParameter(iParamID, iSubscribeToChanges);
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
  DLOG_F(INFO, "CustomView::afterCreate(%d)", fTag);
  auto provider = dynamic_cast<VSTParametersProvider const *>(iDescription->getViewFactory());
  if(provider)
    initParameters(provider->getVSTParameters());
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
  if(fParameters)
    registerParameters();
}
}
}
}

