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
  virtual MaxLevel getMaxLevel() const { return MaxLevel{}; }

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

using MaxLevelAccessor = MaxLevel (HistoryView::*)() const;

/**
 * TMaxLevelView => getMaxLevel is generic
 */
template<MaxLevelAccessor Accessor>
class TMaxLevelView : public MaxLevelView
{
public:
  // Constructor
  explicit TMaxLevelView(const CRect &size) : MaxLevelView{size} {}

  // Deleted CC
  TMaxLevelView(const TMaxLevelView &c) = delete;

  // getMaxLevel
  MaxLevel getMaxLevel() const override {return (this->*Accessor)(); }

  CLASS_METHODS_NOCOPY(TMaxLevelView<Accessor>, MaxLevelView)
};


///////////////////////////////////////////
// MaxLevelSinceResetView
///////////////////////////////////////////
using MaxLevelSinceResetView = TMaxLevelView<&HistoryView::getMaxLevelSinceReset>;
class MaxLevelSinceResetViewCreator : public CustomViewCreator<MaxLevelSinceResetView>
{
public:
  explicit MaxLevelSinceResetViewCreator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
    CustomViewCreator(iViewName, iDisplayName)
  {
    registerAttributes(MaxLevelView::Creator());
  }
};

///////////////////////////////////////////
// MaxLevelInWindowView
///////////////////////////////////////////
using MaxLevelInWindowView = TMaxLevelView<&HistoryView::getMaxLevelInWindow>;
class MaxLevelInWindowViewCreator : public CustomViewCreator<MaxLevelInWindowView>
{
public:
  explicit MaxLevelInWindowViewCreator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
    CustomViewCreator(iViewName, iDisplayName)
  {
    registerAttributes(MaxLevelView::Creator());
  }
};

///////////////////////////////////////////
// MaxLevelForSelectionView
///////////////////////////////////////////
using MaxLevelForSelectionView = TMaxLevelView<&HistoryView::getMaxLevelForSelection>;
class MaxLevelForSelectionViewCreator : public CustomViewCreator<MaxLevelForSelectionView>
{
public:
  explicit MaxLevelForSelectionViewCreator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
    CustomViewCreator(iViewName, iDisplayName)
  {
    registerAttributes(MaxLevelView::Creator());
  }
};


/**
 * Handles the max level text label */
class MaxLevelState : public VSTViewState<MaxLevelView>
{
public:
  explicit MaxLevelState(std::shared_ptr<HistoryState> iHistoryState) : fHistoryState{std::move(iHistoryState)}
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

private:
  friend class MaxLevelView;

  void updateView() const;

  std::shared_ptr<HistoryState> fHistoryState;
};

}
}
}