#include "GainView.h"
#include "../VAC6CIDs.h"
#include "DrawContext.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// GainView::registerParameters
///////////////////////////////////////////
void GainView::registerParameters()
{
  fGain1Parameter = registerVSTParameter<GainParameter>(EVAC6ParamID::kGain1);
  fGain2Parameter = registerVSTParameter<GainParameter>(EVAC6ParamID::kGain2);
  fBypassParameter = registerBooleanParameter(EVAC6ParamID::kBypass);
}

///////////////////////////////////////////
// GainView::draw
///////////////////////////////////////////
void GainView::draw(CDrawContext *iContext)
{
  CustomView::draw(iContext);

  auto rdc = GUI::RelativeDrawContext{this, iContext};

  StringDrawContext sdc{};
  sdc.fHoriTxtAlign = kCenterText;
  sdc.fTextInset = {2, 2};
  sdc.fFontColor = getFontColor();
  sdc.fFont = fFont;

  Gain gain = fBypassParameter->getValue() ? Gain{} : Gain{fGain1Parameter->getValue().getValue() * fGain2Parameter->getValue().getValue()};

  rdc.drawString(toDbString(gain.getValue()), sdc);

  if(fBypassParameter->getValue())
    rdc.fillRect(0,0, getWidth(), getHeight(), CColor{128,128,128,100});

}

GainView::Creator __gGainViewCreator("pongasoft::Gain", "pongasoft - Gain");

}
}
}
