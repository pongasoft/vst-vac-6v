#pragma once

#include "VAC6Views.h"
#include "VSTViewState.h"
#include "../Messaging.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace VSTGUI;
using namespace Common;
using namespace GUI;

class MaxLevelState;

class MaxLevelView : public HistoryView
{
public:
  // Constructor
  explicit MaxLevelView(const CRect &size);

  MaxLevelView(const MaxLevelView &c) = default;

  // setState
  void setState(MaxLevelState *iState)
  {
    fState = iState;
  }

  const CColor &getNoDataColor() const
  {
    return fNoDataColor;
  }

  void setNoDataColor(const CColor &iNoDataColor)
  {
    fNoDataColor = iNoDataColor;
  }

  // draw => does the actual drawing job
  void draw(CDrawContext *iContext) override;

  CLASS_METHODS(MaxLevelView, HistoryView)

protected:
  MaxLevelState *fState{nullptr};

  CColor fNoDataColor{};
};

/**
 * Handles the max level text label */
class MaxLevelState : public VSTViewState<MaxLevelView>
{
public:
  MaxLevelState() : fMaxLevel{}
  {}

  void setMaxLevel(MaxLevel const &maxLevel);

  void afterAssign() override
  {
    fView->setState(this);
    updateView();
  };

  void beforeUnassign() override
  {
    fView->setState(nullptr);
  }

  void onMessage(Message const &message);

private:
  friend class MaxLevelView;

  void updateView() const;

  MaxLevel fMaxLevel;
};

/**
 * The factory for MaxLevelView
 */
class MaxLevelViewCreator : public CustomViewCreator<MaxLevelView>
{
public:
  explicit MaxLevelViewCreator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
    CustomViewCreator(iViewName, iDisplayName)
  {
    registerAttributes(HistoryViewCreator());
    registerColorAttribute("no-data-color",
                           &MaxLevelView::getNoDataColor,
                           &MaxLevelView::setNoDataColor);
  }
};

}
}
}