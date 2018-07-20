#pragma once

#include "CustomView.h"
#include "../VAC6Model.h"
#include "../Utils.h"
#include "DrawContext.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace VSTGUI;
using namespace Common;
using namespace GUI;

class LCDScrollbarView : public CustomView
{
public:
  // Constructor
  explicit LCDScrollbarView(const CRect &size);

  // deleting copy constructor
  LCDScrollbarView(const LCDScrollbarView &c) = delete;

  // get/setScrollbarMinSize
  int32_t getScrollbarMinSize() const { return fScrollbarMinSize; }
  void setScrollbarMinSize(int32_t iScrollbarMinSize) { fScrollbarMinSize = iScrollbarMinSize; }

  // get/setHandleColor
  CColor const& getHandleColor() const { return fHandleColor; }
  void setHandleColor(CColor const &iHandleColor) { fHandleColor = iHandleColor; }

public:
  // draw => does the actual drawing job
  void draw(CDrawContext *iContext) override;

  void registerParameters() override;

  // onMouseDown
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;

  // onMouseMoved
  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override;

  // onMouseUp
  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override;

  // onMouseCancel
  CMouseEventResult onMouseCancel() override;

public:
  CLASS_METHODS_NOCOPY(LCDScrollbarView, CustomView)

protected:
  struct ZoomBox
  {
    RelativeCoord fMinCenter;
    RelativeCoord fMaxCenter;
    RelativeCoord fCenter;
    CCoord fHalfWidth;

    bool isFull() const { return fMinCenter == fMaxCenter; }

    RelativeCoord getLeft() const { return fCenter - fHalfWidth; }

    RelativeCoord getRight() const { return getLeft() + getWidth(); }

    CCoord getWidth() const { return fHalfWidth * 2.0; }

    RelativeCoord computeCenter(double iPercent) const
    {
      return Utils::Lerp<RelativeCoord>(fMinCenter, fMaxCenter).computeY(iPercent);
    }

    double computePercent() const
    {
      return Utils::Lerp<double>(fMinCenter, fMaxCenter).computeX(fCenter);
    }

    void move(CCoord iDeltaX)
    {
      fCenter = Utils::clamp(fCenter + iDeltaX, fMinCenter, fMaxCenter);
    }

  };
protected:
  ZoomBox computeZoomBox() const;

protected:
  CColor fHandleColor{kWhiteCColor};
  int32_t fScrollbarMinSize{17};

  std::unique_ptr<BooleanParameter> fLCDLiveViewParameter{nullptr};

  std::unique_ptr<PercentParameter> fLCDInputHistoryOffsetParameter{nullptr};

  using LCDZoomFactorXParameter = VSTParameter<LCDZoomFactorXParamConverter>;
  std::unique_ptr<LCDZoomFactorXParameter> fLCDZoomFactorXParameter{nullptr};

  std::unique_ptr<PercentParameter::Editor> fLCDInputHistoryOffsetEditor{nullptr};
  std::unique_ptr<ZoomBox> fStartDragGestureZoomBox{nullptr};
  RelativeCoord fStarDragGestureX{-1.0};

public:
  class Creator : public CustomViewCreator<LCDScrollbarView, CustomView>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerColorAttribute("handle-color", &LCDScrollbarView::getHandleColor, &LCDScrollbarView::setHandleColor);
      registerIntAttribute("scrollbar-min-size", &LCDScrollbarView::getScrollbarMinSize, &LCDScrollbarView::setScrollbarMinSize);
    }
  };

};

}
}
}