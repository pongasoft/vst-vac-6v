#pragma once

#include "../VAC6Model.h"
#include "CustomDisplayView.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace GUI;

/**
 * Base class to LCD and MaxLevel
 */
class HistoryView : public CustomDisplayView
{
public:
  explicit HistoryView(const CRect &size) : CustomDisplayView(size)
  {};

  HistoryView(const HistoryView &c) = default;

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

  CLASS_METHODS(HistoryView, CustomDisplayView)

protected:

  // computeColor
  const CColor &computeColor(SoftClippingLevel iLevel, double iSample) const;

  CColor fLevelStateOkColor{};
  CColor fLevelStateSoftClippingColor{};
  CColor fLevelStateHardClippingColor{};
};

/**
 * The factory for HistoryView
 */
class HistoryViewCreator : public CustomViewCreator<HistoryView>
{
public:
  explicit HistoryViewCreator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
    CustomViewCreator(iViewName, iDisplayName)
  {
    registerAttributes(CustomDisplayCreator());
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



}
}
}