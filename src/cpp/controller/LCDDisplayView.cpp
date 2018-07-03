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

  HistoryState::onMessage(message);

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

  bool leftChannelOn = fState->fLCDData.fLeftChannel.fOn;
  bool rightChannelOn = fState->fLCDData.fRightChannel.fOn;

  if(leftChannelOn || rightChannelOn)
  {
    auto maxLevel = getMaxLevel();

    int lcdInputX = maxLevel.fIndex;
    RelativeCoord lcdInputY = -1; // used when paused

    // display every sample in the array as a vertical line (from the bottom)
    for(int i = 0; i < MAX_ARRAY_SIZE; i++)
    {
      double displayValue;


      TSample leftSample = leftChannelOn ? fState->fLCDData.fLeftChannel.fSamples[i] : 0;
      TSample rightSample = rightChannelOn ? fState->fLCDData.fRightChannel.fSamples[i] : 0;

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
    if(!fLCDLiveViewParameter->getValue() && lcdInputX >= 0)
    {
      constexpr auto offset = 3.0;
      rdc.fillRect(RelativeRect{lcdInputX - 5.0, 0, lcdInputX + 5.0, height}, WHITE_COLOR_40);
      rdc.fillRect(RelativeRect{lcdInputX - 3.0, 0, lcdInputX + 3.0, height}, WHITE_COLOR_60);
      rdc.drawLine(lcdInputX, 0, lcdInputX, height, WHITE_COLOR);
      rdc.drawLine(0, lcdInputY, width, lcdInputY, WHITE_COLOR_60);
      rdc.fillAndStrokeRect(RelativeRect{lcdInputX - offset, lcdInputY - offset, lcdInputX + offset, lcdInputY + offset}, RED_COLOR, WHITE_COLOR);
    }
    else
    {
      if(fMaxLevelFollow->getValue() && lcdInputX >= 0)
      {
        constexpr auto offset = 3.0;
        rdc.fillAndStrokeRect(RelativeRect{lcdInputX - offset, lcdInputY - offset, lcdInputX + offset, lcdInputY + offset}, RED_COLOR, WHITE_COLOR);
      }
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

  if(fLCDLiveViewParameter->getValue())
  {
    fLCDLiveViewParameter->setValue(false);
  }

  fLCDInputXEditor = fLCDInputXParameter->edit(static_cast<int>(relativeWhere.x));

  return kMouseEventHandled;
}

///////////////////////////////////////////
// LCDDisplayView::onMouseMoved
///////////////////////////////////////////
CMouseEventResult LCDDisplayView::onMouseMoved(CPoint &where, const CButtonState &buttons)
{
  RelativeView rv(this);
  RelativePoint relativeWhere = rv.fromAbsolutePoint(where);

  if(fLCDInputXEditor)
  {
    fLCDInputXEditor->setValue(static_cast<int>(relativeWhere.x));
    return kMouseEventHandled;
  }

  return kMouseEventNotHandled;
}

///////////////////////////////////////////
// LCDDisplayView::onMouseUp
///////////////////////////////////////////
CMouseEventResult LCDDisplayView::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  RelativeView rv(this);
  RelativePoint relativeWhere = rv.fromAbsolutePoint(where);

  if(fLCDInputXEditor)
  {
    fLCDInputXEditor->commit(static_cast<int>(relativeWhere.x));
    fLCDInputXEditor = nullptr;
    return kMouseEventHandled;
  }

  return kMouseEventNotHandled;
}

///////////////////////////////////////////
// LCDDisplayView::onMouseCancel
///////////////////////////////////////////
CMouseEventResult LCDDisplayView::onMouseCancel()
{
  if(fLCDInputXEditor)
  {
    fLCDInputXEditor->rollback();
    fLCDInputXEditor = nullptr;
    return kMouseEventHandled;
  }

  return kMouseEventNotHandled;
}

///////////////////////////////////////////
// LCDDisplayView::registerParameters
///////////////////////////////////////////
void LCDDisplayView::registerParameters()
{
  HistoryView::registerParameters();
  fMaxLevelFollow = registerBooleanParameter(EVAC6ParamID::kMaxLevelFollow);
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

///////////////////////////////////////////
// LCDDisplayView::setState
///////////////////////////////////////////
void LCDDisplayView::setState(LCDDisplayState *iState)
{
  HistoryView::setState(iState);
  fState = iState;
}


LCDDisplayView::Creator __gLCDDisplayViewCreator("pongasoft::LCDDisplay", "pongasoft - LCD Display");

}
}
}