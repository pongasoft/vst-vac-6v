#pragma once

#include <pongasoft/VST/GUI/Views/ScrollbarView.h>
#include "../VAC6Plugin.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace VSTGUI;
using namespace Common;
using namespace GUI;
using namespace GUI::Views;

class LCDScrollbarView : public PluginView<ScrollbarView, VAC6GUIState>
{
public:
  // Constructor
  explicit LCDScrollbarView(const CRect &iSize) : PluginView(iSize) {};

public:
  void registerParameters() override;

  // onMouseDown
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;

protected:
  GUIVstBooleanParam fLCDLiveViewParameter{nullptr};

public:
  using Creator = CustomViewCreator<LCDScrollbarView, ScrollbarView>;
};

}
}
}