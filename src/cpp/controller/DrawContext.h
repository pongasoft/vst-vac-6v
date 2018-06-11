#pragma once

#include <vstgui4/vstgui/lib/cdrawcontext.h>
#include <vstgui4/vstgui/lib/cview.h>

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

/**
 * Encapsulates the draw context provided by VSTGUI to reason in relative coordinates (0,0) is top,left
 */
class RelativeDrawContext
{
public:
  RelativeDrawContext(CView *iView, CDrawContext *iDrawContext) : fView{iView}, fDrawContext{iDrawContext}
  {
    auto viewSize = fView->getViewSize();
    fOriginX = viewSize.left;
    fOriginY = viewSize.top;
  }

  void drawLine(CCoord iX1, CCoord iY1, CCoord iX2, CCoord iY2, CColor const &color)
  {
    fDrawContext->setFrameColor(color);
    fDrawContext->drawLine(toPoint(iX1, iY1), toPoint(iX2, iY2));
  }

private:
  inline CPoint toPoint(CCoord x, CCoord y)
  {
    return CPoint{x + fOriginX, y + fOriginY};
  }

private:
  CView *fView;
  CDrawContext *fDrawContext;
  CCoord fOriginX;
  CCoord fOriginY;
};

}
}
}