#pragma once

#include "../VAC6Model.h"
#include "../Messaging.h"
#include "CustomView.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace GUI;
using namespace Common;

using SoftClippingLevelParameter = VSTParameter<SoftClippingLevel, SoftClippingLevel::denormalize, SoftClippingLevel::normalize>;
using MaxLevelModeParameter = VSTParameter<MaxLevelMode, MaxLevelModeParamConverter::denormalize, MaxLevelModeParamConverter::normalize>;

class HistoryState;

/**
 * Base class to LCD and MaxLevel
 */
class HistoryView : public CustomView
{
public:
  explicit HistoryView(const CRect &size) : CustomView(size)
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

  // setState
  void setState(HistoryState *iState)
  {
    fState = iState;
  }

  CLASS_METHODS_NOCOPY(HistoryView, CustomView)

protected:
  MaxLevel getMaxLevel() const;

protected:

  // computeColor
  const CColor &computeColor(SoftClippingLevel iLevel, double iSample) const;

  CColor fLevelStateOkColor{};
  CColor fLevelStateSoftClippingColor{};
  CColor fLevelStateHardClippingColor{};

  // the state
  HistoryState *fState{nullptr};


  std::unique_ptr<BooleanParameter> fLCDLiveViewParameter{nullptr};
  std::unique_ptr<SoftClippingLevelParameter> fSoftClippingLevelParameter;
  std::unique_ptr<MaxLevelModeParameter> fMaxLevelModeParameter;

  using LCDInputXParameter = DiscreteParameter<MAX_LCD_INPUT_X>;
  std::unique_ptr<LCDInputXParameter> fLCDInputXParameter{nullptr};

public:
  class Creator : public CustomViewCreator<HistoryView>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerAttributes(CustomView::Creator());
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
class HistoryState
{
public:
  HistoryState() : fLCDData{}
  {}

  virtual void onMessage(Message const &message);

  friend class HistoryView;

protected:

  LCDData fLCDData;

  MaxLevel fMaxLevelInWindow{};
  MaxLevel fMaxLevelSinceReset{};
};

}
}
}