#include "LCDDisplayView.h"
#include "../Clock.h"
#include <vstgui4/vstgui/lib/controls/ccontrol.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

const CColor WHITE_COLOR_40 = CColor{255,255,255,40};
const CColor WHITE_COLOR_60 = CColor{255,255,255,60};

///////////////////////////////////////////
// LCDDisplayState::onMessage
///////////////////////////////////////////
void LCDDisplayState::onMessage(Message const &message)
{
  auto previousWindowSize = fLCDData.fWindowSizeInMillis;
  fLCDData.fWindowSizeInMillis = message.getInt(LCDDATA_WINDOW_SIZE_MS_ATTR, fLCDData.fWindowSizeInMillis);

  fLCDData.fLeftChannelOn = message.getBinary(LCDDATA_LEFT_SAMPLES_ATTR, fLCDData.fLeftSamples, MAX_ARRAY_SIZE) > -1;
  fLCDData.fRightChannelOn = message.getBinary(LCDDATA_RIGHT_SAMPLES_ATTR, fLCDData.fRightSamples, MAX_ARRAY_SIZE) > -1;

  long now = Clock::getCurrentTimeMillis();

  if(previousWindowSize != fLCDData.fWindowSizeInMillis)
  {
    char text[256];
    sprintf(text, "Zoom: %.1fs", fLCDData.fWindowSizeInMillis / 1000.0);
    fLCDZoomFactorXMessage = std::make_unique<LCDMessage>(UTF8String(text), now);
  }

  if(fLCDSoftClippingLevelMessage)
  {
    if(fLCDSoftClippingLevelMessage->update(now))
    {
      fLCDSoftClippingLevelMessage = nullptr;
    }
  }

  if(fLCDZoomFactorXMessage)
  {
    if(fLCDZoomFactorXMessage->update(now))
    {
      fLCDZoomFactorXMessage = nullptr;
    }
  }

  updateView();
}

///////////////////////////////////////////
// LCDDisplayState::onSoftClippingLevelChange
///////////////////////////////////////////
void LCDDisplayState::onSoftClippingLevelChange(SoftClippingLevel const &iNewValue)
{
  long now = Clock::getCurrentTimeMillis();

  char text[256];
  sprintf(text, "%+.2fdB", iNewValue.getValueInDb());

  fLCDSoftClippingLevelMessage = std::make_unique<LCDMessage>(UTF8String(text), now);
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
  HistoryView::draw(iContext);

  if(fState == nullptr)
    return;

  auto rdc = GUI::RelativeDrawContext{this, iContext};

  auto height = getViewSize().getHeight();
  auto width = getViewSize().getWidth();
  RelativeCoord left = 0;

  bool leftChannelOn = fState->fLCDData.fLeftChannelOn;
  bool rightChannelOn = fState->fLCDData.fRightChannelOn;

  if(leftChannelOn || rightChannelOn)
  {
    int lcdInputX = fLCDInputXParameter->getValue();
    RelativeCoord lcdInputY = -1; // used when paused

    // display every sample in the array as a vertical line (from the bottom)
    for(int i = 0; i < MAX_ARRAY_SIZE; i++)
    {
      double displayValue;

      TSample leftSample = leftChannelOn ? fState->fLCDData.fLeftSamples[i] : 0;
      TSample rightSample = rightChannelOn ? fState->fLCDData.fRightSamples[i] : 0;

      TSample sample = std::max(leftSample, rightSample);
      RelativeCoord top = height;

      if(sample >= Common::Sample64SilentThreshold)
      {
        if(sample < MIN_AUDIO_SAMPLE) // a single pixel for -60dB to silent
          displayValue = 1;
        else
        {
          displayValue = toDisplayValue(sample, height);
        }

        top = height - displayValue;

        // color of the sample depends on its level
        CColor const &color = computeColor(fSoftClippingLevelParameter->getValue(), sample);

        rdc.drawLine(left, top, left, height, color);

      }

      if(lcdInputX == i)
        lcdInputY = top;

      left++;
    }

    // in pause mode we draw the selection
    if(!fLCDLiveViewParameter->getValue())
    {
      constexpr auto offset = 3.0;
      rdc.fillRect(RelativeRect{lcdInputX - 5.0, 0, lcdInputX + 5.0, height}, WHITE_COLOR_40);
      rdc.fillRect(RelativeRect{lcdInputX - 3.0, 0, lcdInputX + 3.0, height}, WHITE_COLOR_60);
      rdc.drawLine(lcdInputX, 0, lcdInputX, height, WHITE_COLOR);
      rdc.drawLine(0, lcdInputY, width, lcdInputY, WHITE_COLOR_60);
      rdc.fillAndStrokeRect(RelativeRect{lcdInputX - offset, lcdInputY - offset, lcdInputX + offset, lcdInputY + offset}, RED_COLOR, WHITE_COLOR);
    }
  }

  // display the soft clipping level line (which is controlled by a knob)
  auto softClippingDisplayValue = toDisplayValue(fSoftClippingLevelParameter->getValue().getValueInSample(), height);
  auto top = height - softClippingDisplayValue;

  // draw the soft clipping line
  rdc.drawLine(0, top, getWidth(), top, getSoftClippingLevelColor());


  // TODO handle font: use default?
  // print the level
  if(fState->fLCDSoftClippingLevelMessage)
  {
    StringDrawContext sdc{};
    sdc.fStyle |= kShadowText;
    sdc.fHoriTxtAlign = kLeftText;
    sdc.fTextInset = {2, 2};
    sdc.fFontColor = fState->fLCDSoftClippingLevelMessage->fColor;
    sdc.fShadowColor = BLACK_COLOR;
    sdc.fShadowColor.alpha = fState->fLCDSoftClippingLevelMessage->fColor.alpha;

    auto textTop = top + 3;
    rdc.drawString(fState->fLCDSoftClippingLevelMessage->fText, RelativeRect{0, textTop, MAX_ARRAY_SIZE, textTop + 20}, sdc);

  }

  if(fState->fLCDZoomFactorXMessage)
  {
    StringDrawContext sdc{};
    sdc.fHoriTxtAlign = kCenterText;
    sdc.fTextInset = {2, 2};
    sdc.fFontColor = fState->fLCDZoomFactorXMessage->fColor;

    rdc.drawString(fState->fLCDZoomFactorXMessage->fText, RelativeRect{0, 0, MAX_ARRAY_SIZE, 20}, sdc);
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

  if(fLCDLiveViewParameter->getValue())
  {
    DLOG_F(INFO, "LCDDisplayView::onMouseDown() => stopping");
    fLCDLiveViewParameter->setValue(false);
  }

  fLCDInputXParameter->setValue(static_cast<int>(relativeWhere.x));

  return kMouseEventHandled;
}

///////////////////////////////////////////
// LCDDisplayView::registerParameters
///////////////////////////////////////////
void LCDDisplayView::registerParameters()
{
  HistoryView::registerParameters();
  fLCDLiveViewParameter = registerBooleanParameter(EVAC6ParamID::kLCDLiveView, false);
  fLCDInputXParameter = registerDiscreteParameter<MAX_LCD_INPUT_X>(EVAC6ParamID::kLCDInputX, false);
}

///////////////////////////////////////////
// LCDDisplayView::onParameterChange
///////////////////////////////////////////
void LCDDisplayView::onParameterChange(ParamID iParamID, ParamValue iNormalizedValue)
{
  CustomView::onParameterChange(iParamID, iNormalizedValue);

  switch(iParamID)
  {
    case EVAC6ParamID::kSoftClippingLevel:
      fState->onSoftClippingLevelChange(fSoftClippingLevelParameter->getValue());
      break;

    default:
      break;
  }
}


LCDDisplayView::Creator __gLCDDisplayViewCreator("pongasoft::LCDDisplay", "pongasoft - LCD Display");

}
}
}