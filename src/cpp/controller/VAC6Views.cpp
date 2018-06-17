#include "VAC6Views.h"
#include "../AudioUtils.h"
#include "DrawContext.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

const CColor LevelStateOk_Color{0, 255, 0};
const CColor LevelStateSoftClipping_Color{248, 122, 0};
const CColor LevelStateHardClipping_Color{255, 0, 0};

/**
 * Determine the color based on the value of the sample relative to the soft clipping level
 */
inline CColor const &computeColor(SoftClippingLevel const &iSofClippingLevel, TSample iSample)
{
  CColor const &color =
    iSample > HARD_CLIPPING_LEVEL ? LevelStateHardClipping_Color :
    iSample > iSofClippingLevel.getValueInSample() ? LevelStateSoftClipping_Color :
    LevelStateOk_Color;

  return color;
}

const CColor MaxLevelView_NoData_Color{200, 200, 200};

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
    TSample max = std::max(fMaxLevel.fLeftValue, fMaxLevel.fRightValue);

    char text[256];
    if(max > 0)
    {
      const auto &color = computeColor(fMaxLevel.fSoftClippingLevel, max);
      if(max >= Common::Sample64SilentThreshold)
        sprintf(text, "%+.2f", sampleToDb(max));
      else
        sprintf(text, "-oo");
      fView->setFontColor(color);
    }
    else
    {
      sprintf(text, "---.--");
      fView->setFontColor(MaxLevelView_NoData_Color);
    }

    fView->setText(text);
  }
}

///////////////////////////////////////////
// MaxLevelView::onMessage
///////////////////////////////////////////
void MaxLevelView::onMessage(Message const &message)
{
  MaxLevel maxLevel{};

  maxLevel.fSoftClippingLevel = SoftClippingLevel{message.getFloat(MAX_LEVEL_SOFT_CLIPPING_LEVEL_ATTR,
                                                                   fMaxLevel.fSoftClippingLevel.getValueInSample())};
  maxLevel.fLeftValue = message.getFloat(MAX_LEVEL_LEFT_VALUE_ATTR, -1);
  maxLevel.fRightValue = message.getFloat(MAX_LEVEL_RIGHT_VALUE_ATTR, -1);

  setMaxLevel(maxLevel);
}

// TODO => how to read this from VAC6.uidesc?
const CColor LCDView_SoftClippingLevel_Color{200, 200, 200, 123};

///////////////////////////////////////////
// LCDView::onMessage
///////////////////////////////////////////
void LCDView::onMessage(Message const &message)
{
  fLCDData.fSoftClippingLevel = SoftClippingLevel{message.getFloat(LCDDATA_SOFT_CLIPPING_LEVEL_ATTR,
                                                                   fLCDData.fSoftClippingLevel.getValueInSample())};
  fLCDData.fLeftChannelOn = message.getBinary(LCDDATA_LEFT_SAMPLES_ATTR, fLCDData.fLeftSamples, MAX_ARRAY_SIZE) > -1;
  fLCDData.fRightChannelOn = message.getBinary(LCDDATA_RIGHT_SAMPLES_ATTR, fLCDData.fRightSamples, MAX_ARRAY_SIZE) > -1;

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

  bool leftChannelOn = fLCDData.fLeftChannelOn;
  bool rightChannelOn = fLCDData.fRightChannelOn;

  if(leftChannelOn || rightChannelOn)
  {
    // display every sample in the array as a vertical line (from the bottom)
    for(int i = 0; i < MAX_ARRAY_SIZE; i++)
    {
      double displayValue;

      TSample leftSample = leftChannelOn ? fLCDData.fLeftSamples[i] : 0;
      TSample rightSample = rightChannelOn ? fLCDData.fRightSamples[i] : 0;

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
        CColor const &color = computeColor(fLCDData.fSoftClippingLevel, sample);

        rdc.drawLine(left, top, left, height, color);
      }

      left++;
    }
  }

  // display the soft clipping level line (which is controlled by a knob)
  auto softClippingDisplayValue = toDisplayValue(fLCDData.fSoftClippingLevel.getValueInSample(), height);
  auto top = height - softClippingDisplayValue;

  rdc.drawLine(0, top, fView->getWidth(), top, LCDView_SoftClippingLevel_Color);
}

}
}
}