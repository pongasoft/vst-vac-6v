#include "LCDDisplayView.h"
#include "../Clock.h"
#include "../AudioUtils.h"
#include <vstgui4/vstgui/lib/controls/ccontrol.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// LCDDisplayState::onMessage
///////////////////////////////////////////
void LCDDisplayState::onMessage(Message const &message)
{
  auto previousSoftClippingLevel = fLCDData.fSoftClippingLevel.getValueInSample();
  fLCDData.fSoftClippingLevel = SoftClippingLevel{message.getFloat(LCDDATA_SOFT_CLIPPING_LEVEL_ATTR,
                                                                   fLCDData.fSoftClippingLevel.getValueInSample())};

  auto previousWindowSize = fLCDData.fWindowSizeInMillis;
  fLCDData.fWindowSizeInMillis = message.getInt(LCDDATA_WINDOW_SIZE_MS_ATTR, fLCDData.fWindowSizeInMillis);

  fLCDData.fLeftChannelOn = message.getBinary(LCDDATA_LEFT_SAMPLES_ATTR, fLCDData.fLeftSamples, MAX_ARRAY_SIZE) > -1;
  fLCDData.fRightChannelOn = message.getBinary(LCDDATA_RIGHT_SAMPLES_ATTR, fLCDData.fRightSamples, MAX_ARRAY_SIZE) > -1;

  long now = Clock::getCurrentTimeMillis();

  if(previousSoftClippingLevel != fLCDData.fSoftClippingLevel.getValueInSample())
  {
    delete fLCDSoftClipingLevelMessage;

    char text[256];
    sprintf(text, "%+.2fdB", fLCDData.fSoftClippingLevel.getValueInDb());

    fLCDSoftClipingLevelMessage = new LCDMessage(UTF8String(text), now);
  }

  if(previousWindowSize != fLCDData.fWindowSizeInMillis)
  {
    delete fLCDZoomFactorXMessage;

    char text[256];
    sprintf(text, "Zoom: %.1fs", fLCDData.fWindowSizeInMillis / 1000.0);

    fLCDZoomFactorXMessage = new LCDMessage(UTF8String(text), now);
  }

  if(fLCDSoftClipingLevelMessage != nullptr)
  {
    if(fLCDSoftClipingLevelMessage->update(now))
    {
      delete fLCDSoftClipingLevelMessage;
      fLCDSoftClipingLevelMessage = nullptr;
    }
  }

  if(fLCDZoomFactorXMessage != nullptr)
  {
    if(fLCDZoomFactorXMessage->update(now))
    {
      delete fLCDZoomFactorXMessage;
      fLCDZoomFactorXMessage = nullptr;
    }
  }

  updateView();
}

///////////////////////////////////////////
// LCDDisplayState::onMessage
///////////////////////////////////////////
void LCDDisplayState::updateView() const
{
  if(fView != nullptr)
  {
    fView->setDirty(true);
  }
}

///////////////////////////////////////////
// LCDDisplayState::afterAssign
///////////////////////////////////////////
void LCDDisplayState::afterAssign()
{
  fView->setState(this);
  updateView();
}

///////////////////////////////////////////
// LCDDisplayState::beforeUnassign
///////////////////////////////////////////
void LCDDisplayState::beforeUnassign()
{
  fView->setState(nullptr);
}

///////////////////////////////////////////
// LCDDisplayView::LCDDisplayView
///////////////////////////////////////////
LCDDisplayView::LCDDisplayView(const CRect &size)
  : HistoryView(size)
{

}

///////////////////////////////////////////
// LCDDisplayView::draw
///////////////////////////////////////////
void LCDDisplayView::draw(CDrawContext *iContext)
{
  CustomDisplayView::draw(iContext);

  if(fState == nullptr)
    return;

  auto rdc = GUI::RelativeDrawContext{this, iContext};

  auto height = getViewSize().getHeight();
  CCoord left = 0;

  bool leftChannelOn = fState->fLCDData.fLeftChannelOn;
  bool rightChannelOn = fState->fLCDData.fRightChannelOn;

  if(leftChannelOn || rightChannelOn)
  {
    // display every sample in the array as a vertical line (from the bottom)
    for(int i = 0; i < MAX_ARRAY_SIZE; i++)
    {
      double displayValue;

      TSample leftSample = leftChannelOn ? fState->fLCDData.fLeftSamples[i] : 0;
      TSample rightSample = rightChannelOn ? fState->fLCDData.fRightSamples[i] : 0;

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
        CColor const &color = computeColor(fState->fLCDData.fSoftClippingLevel, sample);

        rdc.drawLine(left, top, left, height, color);
      }

      left++;
    }
  }

  // display the soft clipping level line (which is controlled by a knob)
  auto softClippingDisplayValue = toDisplayValue(fState->fLCDData.fSoftClippingLevel.getValueInSample(), height);
  auto top = height - softClippingDisplayValue;

  // draw the soft clipping line
  rdc.drawLine(0, top, getWidth(), top, getSoftClippingLevelColor());

  // TODO handle font: use default?
  // print the level
  if(fState->fLCDSoftClipingLevelMessage != nullptr)
  {
    StringDrawContext sdc{};
    sdc.fStyle |= kShadowText;
    sdc.fHoriTxtAlign = kLeftText;
    sdc.fTextInset = {2, 2};
    sdc.fFontColor = fState->fLCDSoftClipingLevelMessage->fColor;
    sdc.fShadowColor = BLACK_COLOR;
    sdc.fShadowColor.alpha = fState->fLCDSoftClipingLevelMessage->fColor.alpha;

    auto textTop = top + 3;
    rdc.drawString(fState->fLCDSoftClipingLevelMessage->fText, CRect{0, textTop, MAX_ARRAY_SIZE, textTop + 20}, sdc);

  }

  if(fState->fLCDZoomFactorXMessage != nullptr)
  {
    StringDrawContext sdc{};
    sdc.fHoriTxtAlign = kCenterText;
    sdc.fTextInset = {2, 2};
    sdc.fFontColor = fState->fLCDZoomFactorXMessage->fColor;

    rdc.drawString(fState->fLCDZoomFactorXMessage->fText, CRect{0, 0, MAX_ARRAY_SIZE, 20}, sdc);
  }
}

///////////////////////////////////////////
// LCDDisplayView::onMouseDown
///////////////////////////////////////////
CMouseEventResult LCDDisplayView::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  RelativeView rv(this);
  RelativePoint relativeWhere = rv.fromAbsolutePoint(where);

  DLOG_F(INFO, "LCDDisplayView::onMouseDown(%f,%f)", relativeWhere.x, relativeWhere.y);
  return CView::onMouseDown(where, buttons);
}


LCDDisplayViewCreator __gLCDDisplayViewCreator("pongasoft::LCDDisplay", "pongasoft - LCD Display");

}
}
}