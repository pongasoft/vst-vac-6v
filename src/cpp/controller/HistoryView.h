#pragma once

#include <pongasoft/VST/Messaging.h>
#include <pongasoft/VST/GUI/Views/CustomView.h>
#include "../VAC6Model.h"
#include "../VAC6Plugin.h"

namespace pongasoft::VST::VAC6 {

using namespace GUI::Views;
using namespace GUI::Params;

using namespace Common;

/**
 * Base class to LCD and MaxLevel
 */
class HistoryView : public StateAwareCustomView<VAC6GUIState>
{
public:
  explicit HistoryView(const CRect &size) : StateAwareCustomView<VAC6GUIState>(size)
  {};

  HistoryView(const HistoryView &c) = delete;

  // get/setLevelStateOkColor
  const CColor &getLevelStateOkColor() const { return fLevelStateOkColor;}
  void setLevelStateOkColor(const CColor &iColor) { fLevelStateOkColor = iColor; }

  // get/setLevelStateSoftClippingColor
  const CColor &getLevelStateSoftClippingColor() const { return fLevelStateSoftClippingColor; }
  void setLevelStateSoftClippingColor(const CColor &iColor) { fLevelStateSoftClippingColor = iColor; }

  // get/setLevelStateHardClippingColor
  const CColor &getLevelStateHardClippingColor() const { return fLevelStateHardClippingColor; }
  void setLevelStateHardClippingColor(const CColor &iColor) { fLevelStateHardClippingColor = iColor; }

  // registerParameters
  void registerParameters() override;

  CLASS_METHODS_NOCOPY(HistoryView, CustomView)

  MaxLevel getMaxLevelSinceReset() const;
  MaxLevel getMaxLevelInWindow() const;

protected:

  // computeColor
  const CColor &computeColor(SoftClippingLevel iLevel, double iSample) const;

  CColor fLevelStateOkColor{};
  CColor fLevelStateSoftClippingColor{};
  CColor fLevelStateHardClippingColor{};

  GUIVstParam<SoftClippingLevel> fSoftClippingLevelParameter;
  GUIVstParam<int> fLCDInputXParameter{nullptr};
  GUIJmbParam<HistoryData> fHistoryDataParam{};

public:
  class Creator : public CustomViewCreator<HistoryView, StateAwareCustomView<VAC6GUIState>>
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

}