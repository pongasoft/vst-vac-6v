#include "VAC6Views.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// HistoryView::HistoryView
///////////////////////////////////////////
HistoryView::HistoryView(const CRect &size, IControlListener *listener, int32_t tag, CBitmap *pBackground)
  : CustomDisplayView(size, listener, tag, pBackground)
{

}

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



}
}
}