#pragma once

#include <vstgui4/vstgui/lib/controls/ctextlabel.h>
#include <pluginterfaces/vst/ivstmessage.h>

#include "VSTViewState.h"
#include "../VAC6Constants.h"
#include "../Messaging.h"
#include "../VAC6Model.h"
#include "CustomDisplayView.h"
#include "DrawContext.h"
#include "../Utils.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace VSTGUI;
using namespace Common;
using namespace Steinberg::Vst;
using namespace GUI;

/**
 * Base class to LCD and MaxLevel
 */
class HistoryView : public CustomDisplayView
{
public:
  HistoryView(const CRect &size, IControlListener *listener, int32_t tag, CBitmap *pBackground);

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