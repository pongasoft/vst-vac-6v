#include <pongasoft/VST/GUI/DrawContext.h>
#include "GainView.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// GainView::registerParameters
///////////////////////////////////////////
void GainView::registerParameters()
{
  fGain1Parameter = registerParam(fParams->fGain1Param);
  fGain2Parameter = registerParam(fParams->fGain2Param);
  fBypassParameter = registerParam(fParams->fBypassParam);
}

///////////////////////////////////////////
// GainView::draw
///////////////////////////////////////////
void GainView::draw(CDrawContext *iContext)
{
  CustomView::draw(iContext);

  auto rdc = GUI::RelativeDrawContext{this, iContext};

  StringDrawContext sdc{};
  sdc.fHorizTxtAlign = kCenterText;
  sdc.fTextInset = {2, 2};
  sdc.fFontColor = getFontColor();
  sdc.fFont = fFont;

  Gain gain = fBypassParameter.getValue() ? Gain{} : Gain{fGain1Parameter.getValue().getValue() * fGain2Parameter.getValue().getValue()};

  rdc.drawString(toDbString(gain.getValue()), sdc);

  if(fBypassParameter)
    rdc.fillRect(0,0, getWidth(), getHeight(), CColor{128,128,128,100});

}

GainView::Creator __gGainViewCreator("VAC6V::Gain", "VAC6V - Gain");

}
}
}
