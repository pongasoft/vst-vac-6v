#include <vstgui4/vstgui/uidescription/iviewcreator.h>
#include <vstgui4/vstgui/lib/cdrawcontext.h>
#include "CustomView.h"


namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

///////////////////////////////////////////
// CustomView::CustomView
///////////////////////////////////////////
CustomView::CustomView(const CRect &size)
  : CView(size),
    fBackColor{0,0,0},
    fParameters{nullptr},
    fParameterConnections{}
{
  DLOG_F(INFO, "CustomView::CustomView()");
  setWantsFocus(true);
}

///////////////////////////////////////////
// CustomView::draw
///////////////////////////////////////////
void CustomView::draw(CDrawContext *iContext)
{
  iContext->setFillColor(getBackColor());
  iContext->drawRect(getViewSize(), kDrawFilled);

  setDirty(false);
}

///////////////////////////////////////////
// CustomView::setBackColor
///////////////////////////////////////////
void CustomView::setBackColor(CColor const &color)
{
  // to force the redraw
  if(fBackColor != color)
  {
    fBackColor = color;
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
  auto parameter = fParameters->getRawParameter(iParamID);

  if(iSubscribeToChanges)
  {
    fParameterConnections[iParamID] = std::move(parameter->connect(this));
  }

  return parameter;
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

CustomView::Creator __gCustomDisplayCreator("pongasoft::CustomDisplay", "pongasoft - Custom Display");
}
}
}

