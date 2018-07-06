#pragma once

#include "HistoryView.h"
#include "VSTViewState.h"
#include "VSTParameters.h"
#include "../Messaging.h"
#include "../VAC6CIDs.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace VSTGUI;
using namespace Common;
using namespace GUI;

class MaxLevelState;

///////////////////////////////////////////
// MaxLevelView
///////////////////////////////////////////
class MaxLevelView : public HistoryView
{
public:
  // Constructor
  explicit MaxLevelView(const CRect &size);

  MaxLevelView(const MaxLevelView &c) = delete;

  // setState
  void setState(MaxLevelState *iState);

  // get/setNoDataColor
  const CColor &getNoDataColor() const { return fNoDataColor;  }
  void setNoDataColor(const CColor &iNoDataColor) { fNoDataColor = iNoDataColor; }

  // getMaxLevel
  MaxLevel getMaxLevel() const;

  // draw => does the actual drawing job
  void draw(CDrawContext *iContext) override;

  CLASS_METHODS_NOCOPY(MaxLevelView, HistoryView)

protected:
  MaxLevelState *fState{nullptr};

  CColor fNoDataColor{};

public:
  class Creator : public CustomViewCreator<MaxLevelView>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerAttributes(HistoryView::Creator());
      registerColorAttribute("no-data-color",
                             &MaxLevelView::getNoDataColor,
                             &MaxLevelView::setNoDataColor);
    }
  };
};

/**
 * Handles the max level text label */
class MaxLevelState : public VSTViewState<MaxLevelView>
{
public:
  enum class Type
  {
    kForSelection,
    kSinceReset,
    kInWindow
  };

public:
  MaxLevelState(Type iType, std::shared_ptr<HistoryState> iHistoryState) :
    fType{iType},
    fHistoryState{std::move(iHistoryState)}
  {}

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

  MaxLevel getMaxLevel(int iLCDInputX) const;

private:
  friend class MaxLevelView;

  void updateView() const;

  Type fType;
  std::shared_ptr<HistoryState> fHistoryState;
};

}
}
}