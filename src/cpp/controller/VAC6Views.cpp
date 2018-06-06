#include "VAC6Views.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// MaxLevelView::MaxLevelView
///////////////////////////////////////////
MaxLevelView::MaxLevelView() : VSTView{}, fMaxLevel{0, kStateOk}
{
}

///////////////////////////////////////////
// MaxLevelView::afterAssign
///////////////////////////////////////////
void MaxLevelView::afterAssign()
{
  updateView();
}

///////////////////////////////////////////
// MaxLevelView::setMaxLevel
///////////////////////////////////////////
void MaxLevelView::setMaxLevel(MaxLevel const &maxLevel)
{
  fMaxLevel = maxLevel;
  updateView();
}

///////////////////////////////////////////
// MaxLevelView::updateView
///////////////////////////////////////////
void MaxLevelView::updateView() const
{
  if(fView != nullptr)
  {
    char text[256];
    sprintf(text, "%.2f / %d", fMaxLevel.fValue, fMaxLevel.fState);
    fView->setText(text);

    switch(fMaxLevel.fState)
    {
      case kStateOk:
        fView->setFontColor(CColor{0, 255, 0});
        break;

      case kStateSoftClipping:
        fView->setFontColor(CColor{248, 122, 0});
        break;

      case kStateHardClipping:
        fView->setFontColor(CColor{255, 0, 0});
        break;

      default:
        // should not be here
        break;
    }
  }
}

///////////////////////////////////////////
// MaxLevelView::onMessage
///////////////////////////////////////////
void MaxLevelView::onMessage(Message const &message)
{
  MaxLevel maxLevel{
    message.getFloat("Value", 0),
    static_cast<EMaxLevelState>(message.getInt("State", 0))
  };

  setMaxLevel(maxLevel);
}

}
}
}