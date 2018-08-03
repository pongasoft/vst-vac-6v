#pragma once

#include <pongasoft/VST/Messaging.h>
#include <pongasoft/VST/GUI/Views/CustomView.h>
#include <pongasoft/VST/GUI/GUIViewState.h>
#include "../VAC6Model.h"
#include "../VAC6Plugin.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace GUI::Views;
using namespace GUI::Params;

using namespace Common;

class HistoryState;

/**
 * Base class to LCD and MaxLevel
 */
class HistoryView : public PluginCustomView<VAC6GUIState>
{
public:
  explicit HistoryView(const CRect &size) : PluginCustomView(size)
  {};

  HistoryView(const HistoryView &c) = delete;

  // getLevelStateOkColor
  const CColor &getLevelStateOkColor() const
  {
    return fLevelStateOkColor;
  }

  // setLevelStateOkColor
  void setLevelStateOkColor(const CColor &iLevelStateOkColor)
  {
    fLevelStateOkColor = iLevelStateOkColor;
  }

  // getLevelStateSoftClippingColor
  const CColor &getLevelStateSoftClippingColor() const
  {
    return fLevelStateSoftClippingColor;
  }

  // setLevelStateSoftClippingColor
  void setLevelStateSoftClippingColor(const CColor &iLevelStateSoftClippingColor)
  {
    fLevelStateSoftClippingColor = iLevelStateSoftClippingColor;
  }

  // getLevelStateHardClippingColor
  const CColor &getLevelStateHardClippingColor() const
  {
    return fLevelStateHardClippingColor;
  }

  // setLevelStateHardClippingColor
  void setLevelStateHardClippingColor(const CColor &iLevelStateHardClippingColor)
  {
    fLevelStateHardClippingColor = iLevelStateHardClippingColor;
  }

  // registerParameters
  void registerParameters() override;

  CLASS_METHODS_NOCOPY(HistoryView, CustomView)

  MaxLevel getMaxLevelSinceReset() const;
  MaxLevel getMaxLevelInWindow() const;

  // setHistoryState
  virtual void setHistoryState(std::shared_ptr<HistoryState> iHistoryState);

protected:

  std::shared_ptr<HistoryState> fHistoryState;

protected:

  // computeColor
  const CColor &computeColor(SoftClippingLevel iLevel, double iSample) const;

  CColor fLevelStateOkColor{};
  CColor fLevelStateSoftClippingColor{};
  CColor fLevelStateHardClippingColor{};

  GUIVstParam<SoftClippingLevel> fSoftClippingLevelParameter;
  GUIVstParam<int> fLCDInputXParameter{nullptr};

public:
  class Creator : public CustomViewCreator<HistoryView, CustomView>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerColorAttribute("level-state-ok-color",
                             &HistoryView::getLevelStateOkColor,
                             &HistoryView::setLevelStateOkColor);
      registerColorAttribute("level-state-soft-clipping-color",
                             &HistoryView::getLevelStateSoftClippingColor,
                             &HistoryView::setLevelStateSoftClippingColor);
      registerColorAttribute("level-state-hard-clipping-color",
                             &HistoryView::getLevelStateHardClippingColor,
                             &HistoryView::setLevelStateHardClippingColor);
    }
  };
};

/**
 * Base class for HistoryState
 */
class HistoryState : public GUI::GUIViewState
{
public:
  HistoryState() : fLCDData{}
  {
    DLOG_F(INFO, "HistoryState()");
  }

  ~HistoryState() override
  {
    DLOG_F(INFO, "~HistoryState()");
  }

  virtual void onMessage(Message const &message);

  MaxLevel getMaxLevelForSelection(int iLCDInputX) const;

  LCDData fLCDData;
  MaxLevel fMaxLevelInWindow{};
  MaxLevel fMaxLevelSinceReset{};
};

}
}
}