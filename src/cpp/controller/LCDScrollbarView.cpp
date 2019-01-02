#include "LCDScrollbarView.h"
#include "../VAC6CIDs.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// LCDScrollbarView::registerParameters
///////////////////////////////////////////
void LCDScrollbarView::registerParameters()
{
  fLCDLiveViewParameter = registerParam(fParams->fLCDLiveViewParam);
  PluginView::registerParameters();
}

///////////////////////////////////////////
// LCDScrollbarView::onMouseDown
///////////////////////////////////////////
CMouseEventResult LCDScrollbarView::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  if(fLCDLiveViewParameter)
  {
    fLCDLiveViewParameter.setValue(false);
  }

  return PluginView::onMouseDown(where, buttons);
}

LCDScrollbarView::Creator __gLCDScrollbarViewCreator("VAC6V::LCDScrollbar", "VAC6V - LCD Scrollbar");

}
}
}