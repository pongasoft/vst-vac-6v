#include "HistoryView.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// HistoryView::computeColor
///////////////////////////////////////////
const CColor &HistoryView::computeColor(SoftClippingLevel iSofClippingLevel, double iSample) const
{
  CColor const &color =
    iSample > HARD_CLIPPING_LEVEL ? getLevelStateHardClippingColor() :
    iSample > iSofClippingLevel.getValueInSample() ? getLevelStateSoftClippingColor() :
    getLevelStateOkColor();

  return color;
}

///////////////////////////////////////////
// HistoryView::registerParameters
///////////////////////////////////////////
void HistoryView::registerParameters()
{
  CustomView::registerParameters();

  fSoftClippingLevelParameter = registerParam(fParams->fSoftClippingLevelParam);
  fLCDInputXParameter = registerParam(fParams->fLCDInputXParam);
  fHistoryDataParam = registerParam(fState->fHistoryData);
}

///////////////////////////////////////////
// HistoryView::getMaxLevelSinceReset
///////////////////////////////////////////
MaxLevel HistoryView::getMaxLevelSinceReset() const
{
  return fHistoryDataParam->fMaxLevelSinceReset;
}

///////////////////////////////////////////
// HistoryView::getMaxLevelInWindow
///////////////////////////////////////////
MaxLevel HistoryView::getMaxLevelInWindow() const
{
  return fHistoryDataParam->fMaxLevelInWindow;
}
}
}
}