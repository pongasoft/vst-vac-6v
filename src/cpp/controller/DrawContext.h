#pragma once

#include <vstgui4/vstgui/lib/cdrawcontext.h>
#include <vstgui4/vstgui/lib/cview.h>

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

const CColor WHITE_COLOR = CColor{255, 255, 255};
const CColor BLACK_COLOR = CColor{0, 0, 0};

struct StringDrawContext
{
  CHoriTxtAlign fHoriTxtAlign{kCenterText};
  int32_t fStyle{0};
  CFontRef fFontID{nullptr};
  CColor fFontColor{WHITE_COLOR};
  CColor fShadowColor{BLACK_COLOR};
  CPoint fTextInset{0, 0};
  CPoint fShadowTextOffset{1., 1.};
  bool fAntialias{true};
};

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

  void drawLine(CCoord x1, CCoord y1, CCoord x2, CCoord y2, CColor const &color)
  {
    fDrawContext->setFrameColor(color);
    fDrawContext->drawLine(toAdjustedPoint(x1, y1), toAdjustedPoint(x2, y2));
  }

  void drawString(UTF8String const &iText, CCoord x, CCoord y, CCoord iHeight, StringDrawContext &iSdc)
  {
    CRect size{x, y, fDrawContext->getStringWidth(iText.getPlatformString()), iHeight};
    drawString(iText, size, iSdc);
  }

  void drawString(UTF8String const &iText, CRect const &fSize, StringDrawContext &iSdc);

private:
  inline CCoord adjustX(CCoord x)
  {
    return x + fOriginX;
  }

  inline CCoord adjustY(CCoord y)
  {
    return y + fOriginY;
  }

  inline CPoint adjustPoint(CPoint const &iPoint)
  {
    return CPoint{adjustX(iPoint.x), adjustY(iPoint.y)};
  }

  inline CPoint toAdjustedPoint(CCoord x, CCoord y)
  {
    return CPoint{x + fOriginX, y + fOriginY};
  }

  inline CRect adjustRect(CRect const &iRect)
  {
    return CRect(adjustPoint(iRect.getTopLeft()), iRect.getSize());
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