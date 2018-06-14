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
    message.getFloat(MAX_LEVEL_VALUE_ATTR, 0),
    static_cast<EMaxLevelState>(message.getInt(MAX_LEVEL_STATE_ATTR, 0))
  };

  setMaxLevel(maxLevel);
}

// TODO => how to read this from VAC6.uidesc?
const CColor LCDView_LevelStateOk_Color{0, 255, 0};
const CColor LCDView_LevelStateSoftClipping_Color{248, 122, 0};
const CColor LCDView_LevelStateHardClipping_Color{255, 0, 0};
const CColor LCDView_SoftClippingLevel_Color{200, 200, 200, 123};

///////////////////////////////////////////
// LCDView::onMessage
///////////////////////////////////////////
void LCDView::onMessage(Message const &message)
{
  fLCDData.fSoftClippingLevel = SoftClippingLevel{message.getFloat(LCDDATA_SOFT_CLIPPING_LEVEL_ATTR,
                                                                   fLCDData.fSoftClippingLevel.getValueInSample())};
  message.getBinary(LCDDATA_LEFT_SAMPLES_ATTR, fLCDData.fLeftSamples, MAX_ARRAY_SIZE);
  message.getBinary(LCDDATA_RIGHT_SAMPLES_ATTR, fLCDData.fRightSamples, MAX_ARRAY_SIZE);

  updateView();
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

  // display every sample in the array as a vertical line (from the bottom)
  for(int i = 0; i < MAX_ARRAY_SIZE; i++)
  {
    double displayValue;

    TSample leftSample = fLCDData.fLeftSamples[i];
    TSample rightSample = fLCDData.fRightSamples[i];

    TSample sample = std::max(leftSample, rightSample);

    if(sample >= Common::Sample64SilentThreshold)
    {
      if(sample < MIN_AUDIO_SAMPLE) // a single pixel for -60dB to silent
        displayValue = 1;
      else
      {
        displayValue = toDisplayValue(sample, height);
      }

      auto top = height - displayValue;

      // color of the sample depends on its level
      CColor const &color =
        sample > HARD_CLIPPING_LEVEL ? LCDView_LevelStateHardClipping_Color :
        sample > fLCDData.fSoftClippingLevel.getValueInSample() ? LCDView_LevelStateSoftClipping_Color :
        LCDView_LevelStateOk_Color;

      rdc.drawLine(left, top, left, height, color);
    }

    left++;
  }

  // display the soft clipping level line (which is controlled by a knob)
  auto softClippingDisplayValue = toDisplayValue(fLCDData.fSoftClippingLevel.getValueInSample(), height);
  auto top = height - softClippingDisplayValue;

  rdc.drawLine(0, top, fView->getWidth(), top, LCDView_SoftClippingLevel_Color);
}

}
}
}