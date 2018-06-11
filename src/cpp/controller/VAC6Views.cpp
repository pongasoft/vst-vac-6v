#include "VAC6Views.h"
#include "../AudioUtils.h"
#include "DrawContext.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

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

///////////////////////////////////////////
// LCDView::onMessage
///////////////////////////////////////////
void LCDView::onMessage(Message const &message)
{
  if(message.getBinary("Value", fLCDData.fSamples, MAX_ARRAY_SIZE) == MAX_ARRAY_SIZE)
  {
    updateView();
  }
}

///////////////////////////////////////////
// LCDView::onMessage
///////////////////////////////////////////
void LCDView::updateView() const
{
  if(fView != nullptr)
  {
    fView->setDirty(true);
  }
}

///////////////////////////////////////////
// LCDView::afterAssign
///////////////////////////////////////////
void LCDView::afterAssign()
{
  auto cb = [this](CustomDisplayView * /* view */, CDrawContext *iContext) -> void {
    draw(iContext);
  };
  fView->setDrawCallback(cb);
  updateView();
}

///////////////////////////////////////////
// LCDView::beforeUnassign
///////////////////////////////////////////
void LCDView::beforeUnassign()
{
  fView->setDrawCallback(nullptr);
}

///////////////////////////////////////////
// LCDView::draw
///////////////////////////////////////////
void LCDView::draw(CDrawContext *iContext) const
{
  auto rdc = GUI::RelativeDrawContext{fView, iContext};

  auto height = fView->getViewSize().getHeight();
  CCoord left = 0;
  for(TSample sample : fLCDData.fSamples)
  {
    double displayValue;

    if(sample >= Common::Sample64SilentThreshold)
    {
      if(sample < MIN_AUDIO_SAMPLE) // a single pixel for -60dB to silent
        displayValue = 1;
      else
      {
        displayValue = toDisplayValue(sample, height);
      }

      auto top = height - displayValue;

      rdc.drawLine(left, top, left, height, CColor{0,255,0});
    }

    left++;
  }
}

}
}
}