#pragma once

#include "CustomView.h"
#include "../VAC6Model.h"

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

  LCDScrollbarView(const LCDScrollbarView &c) = delete;

public:
  // draw => does the actual drawing job
  void draw(CDrawContext *iContext) override;

  void registerParameters() override;

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;

//  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override;
//
//  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override;
//
//  CMouseEventResult onMouseCancel() override;

public:
  CLASS_METHODS_NOCOPY(LCDScrollbarView, CustomView)

protected:

  std::unique_ptr<BooleanParameter> fLCDLiveViewParameter{nullptr};

  std::unique_ptr<PercentParameter> fLCDInputHistoryOffsetParameter{nullptr};

public:
  class Creator : public CustomViewCreator<LCDScrollbarView>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerAttributes(CustomView::Creator());
    }
  };

};

}
}
}