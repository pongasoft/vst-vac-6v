#include "LCDDisplayView.h"
#include "../Clock.h"
#include "../AudioUtils.h"
#include <vstgui4/vstgui/lib/controls/ccontrol.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

const CColor MAX_LEVEL_FOR_SELECTION_LINES_COLOR = CColor{255,255,255,200};
const CColor MAX_LEVEL_SINCE_RESET_COLOR = CColor{255,0,0,220};
const CColor MAX_LEVEL_IN_WINDOW_COLOR = CColor{0,0,255,220};
const CColor MAX_LEVEL_FOR_SELECTION_COLOR = CColor{0,0,0,40};

///////////////////////////////////////////
// LCDDisplayState::onMessage
///////////////////////////////////////////
void LCDDisplayState::onMessage(Message const &message)
{
  long now = Clock::getCurrentTimeMillis();

  if(fWindowSizeInMillis != fHistoryState->fLCDData.fWindowSizeInMillis)
  {
    // TODO use LCDZoomFactorXParamConverter::toString (need parameter)
    char text[256];
    sprintf(text, "Zoom: %.1fs", fHistoryState->fLCDData.fWindowSizeInMillis / 1000.0);
    fLCDZoomFactorXMessage = std::make_unique<LCDMessage>(UTF8String(text), now);
    fWindowSizeInMillis = fHistoryState->fLCDData.fWindowSizeInMillis;
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

  fLCDSoftClippingLevelMessage =
    std::make_unique<LCDMessage>(toDbString(iNewValue.getValueInSample()), now);
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
// LCDDisplayView::drawMaxLevel
///////////////////////////////////////////
void LCDDisplayView::drawMaxLevel(GUI::RelativeDrawContext &iContext,
                                  RelativePoint const &iPoint,
                                  CCoord iHalfSize,
                                  CColor const &iColor)
{
  if(iPoint.x > -1 && iPoint.y > -1 && iPoint.y < getHeight())
  {
    drawMaxLevelNoCheck(iContext, iPoint, iHalfSize, iColor);
  }
}

///////////////////////////////////////////
// LCDDisplayView::drawMaxLevelNoCheck
///////////////////////////////////////////
void LCDDisplayView::drawMaxLevelNoCheck(GUI::RelativeDrawContext &iContext,
                                         RelativePoint const &iPoint,
                                         CCoord iHalfSize,
                                         CColor const &iColor)
{
  iContext.fillAndStrokeRect(RelativeRect{iPoint.x - iHalfSize,
                                          iPoint.y - iHalfSize,
                                          iPoint.x + iHalfSize,
                                          iPoint.y + iHalfSize},
                             iColor,
                             kWhiteCColor);
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

  LCDData &lcdData = fState->fHistoryState->fLCDData;

  bool leftChannelOn = lcdData.fLeftChannel.fOn;
  bool rightChannelOn = lcdData.fRightChannel.fOn;

  if(leftChannelOn || rightChannelOn)
  {

    MaxLevel maxLevelForSelection{-1, fLCDInputXParameter->getValue()};
    RelativePoint maxLevelForSelectionPoint = {static_cast<RelativeCoord>(maxLevelForSelection.fIndex), -1};

    auto maxLevelSinceReset = getMaxLevelSinceReset();
    RelativePoint maxLevelSinceResetPoint = { static_cast<RelativeCoord>(maxLevelSinceReset.fIndex), -1 };

    auto maxLevelInWindow = getMaxLevelInWindow();
    RelativePoint maxLevelInWindowPoint = {static_cast<RelativeCoord>(maxLevelInWindow.fIndex), -1 };

    // display every sample in the array as a vertical line (from the bottom)
    for(int i = 0; i < MAX_ARRAY_SIZE; i++)
    {
      double displayValue;

      TSample leftSample = leftChannelOn ? lcdData.fLeftChannel.fSamples[i] : 0;
      TSample rightSample = rightChannelOn ? lcdData.fRightChannel.fSamples[i] : 0;

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

        top = Utils::clamp<RelativeCoord>(height - displayValue, -0.5, height);

        // color of the sample depends on its level
        CColor const &color = computeColor(fSoftClippingLevelParameter->getValue(), sample);

        rdc.drawLine(left, top, left, height, color);

      }

      if(maxLevelForSelection.fIndex == i)
      {
        maxLevelForSelection.fValue = sample;
        maxLevelForSelectionPoint.y = top;
      }

      if(maxLevelSinceResetPoint.x == i)
        maxLevelSinceResetPoint.y = top;

      if(maxLevelInWindowPoint.x == i)
        maxLevelInWindowPoint.y = top;

      left++;
    }

    if(fMaxLevelInWindowMarker->getValue())
    {
      // on top of each other => make bigger
      CCoord size = 3.0;
      if(maxLevelInWindowPoint.x == maxLevelSinceResetPoint.x && fMaxLevelSinceResetMarker->getValue())
      {
        size = 5.0;
      }
      drawMaxLevel(rdc, maxLevelInWindowPoint, size, MAX_LEVEL_IN_WINDOW_COLOR);
    }

    if(fMaxLevelSinceResetMarker->getValue())
    {
      drawMaxLevel(rdc, maxLevelSinceResetPoint, 3.0, MAX_LEVEL_SINCE_RESET_COLOR);
    }

    if(maxLevelForSelectionPoint.x > -1)
    {
      RelativeCoord x = maxLevelForSelectionPoint.x;
      RelativeCoord y = maxLevelForSelectionPoint.y;
      drawMaxLevelNoCheck(rdc, maxLevelForSelectionPoint, 7.0, MAX_LEVEL_FOR_SELECTION_COLOR);
      rdc.drawLine(x, 0, x, height, MAX_LEVEL_FOR_SELECTION_LINES_COLOR);
      rdc.drawLine(0, y, width, y, MAX_LEVEL_FOR_SELECTION_LINES_COLOR);
    }
  }

  // display the soft clipping level line (which is controlled by a knob)
  auto softClippingDisplayValue = toDisplayValue(fSoftClippingLevelParameter->getValue().getValueInSample(), height);
  auto top = height - softClippingDisplayValue;

  // draw the soft clipping line
  rdc.drawLine(0, top, getWidth(), top, getSoftClippingLevelColor());


  // print the level
  if(fState->fLCDSoftClippingLevelMessage)
  {
    StringDrawContext sdc{};
    sdc.fStyle |= kShadowText;
    sdc.fHoriTxtAlign = kLeftText;
    sdc.fTextInset = {2, 2};
    sdc.fFontColor = fState->fLCDSoftClippingLevelMessage->fColor;
    sdc.fFont = fFont;
    sdc.fShadowColor = kBlackCColor;
    sdc.fShadowColor.alpha = fState->fLCDSoftClippingLevelMessage->fColor.alpha;

    auto textTop = top + 3;
    rdc.drawString(fState->fLCDSoftClippingLevelMessage->fText,
                   RelativeRect{0, textTop, static_cast<RelativeCoord>(MAX_ARRAY_SIZE), textTop + 20}, sdc);
  }

  if(fState->fLCDZoomFactorXMessage)
  {
    StringDrawContext sdc{};
    sdc.fHoriTxtAlign = kCenterText;
    sdc.fTextInset = {2, 2};
    sdc.fFontColor = fState->fLCDZoomFactorXMessage->fColor;
    sdc.fFont = fFont;

    rdc.drawString(fState->fLCDZoomFactorXMessage->fText,
                   RelativeRect{0, 0, static_cast<RelativeCoord>(MAX_ARRAY_SIZE), 20}, sdc);
  }
}

///////////////////////////////////////////
// LCDDisplayView::computeLCDInputX
///////////////////////////////////////////
int LCDDisplayView::computeLCDInputX(CPoint &where) const
{
  RelativeView rv(this);
  return Utils::clamp(rv.fromAbsolutePoint(where).x, 0, MAX_LCD_INPUT_X);
}

///////////////////////////////////////////
// LCDDisplayView::onMouseDown
///////////////////////////////////////////
CMouseEventResult LCDDisplayView::onMouseDown(CPoint &where, const CButtonState &buttons)
{
  if(fLCDLiveViewParameter->getValue())
  {
    fLCDLiveViewParameter->setValue(false);
  }

  fLCDInputXEditor = fLCDInputXParameter->edit(computeLCDInputX(where));

  return kMouseEventHandled;
}

///////////////////////////////////////////
// LCDDisplayView::onMouseMoved
///////////////////////////////////////////
CMouseEventResult LCDDisplayView::onMouseMoved(CPoint &where, const CButtonState &buttons)
{
  if(fLCDInputXEditor)
  {
    fLCDInputXEditor->setValue(computeLCDInputX(where));
    return kMouseEventHandled;
  }

  return kMouseEventNotHandled;
}

///////////////////////////////////////////
// LCDDisplayView::onMouseUp
///////////////////////////////////////////
CMouseEventResult LCDDisplayView::onMouseUp(CPoint &where, const CButtonState &buttons)
{
  if(fLCDInputXEditor)
  {
    fLCDInputXEditor->commit(computeLCDInputX(where));
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
  fMaxLevelSinceResetMarker = registerGUIParam(fParams->fSinceResetMarkerParam);
  fMaxLevelInWindowMarker = registerGUIParam(fParams->fInWindowMarkerParam);
  fLCDLiveViewParameter = registerGUIParam(fParams->fLCDLiveViewParam);
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
  fState = iState;
  if(iState)
  {
    fHistoryState = iState->fHistoryState;
  }
  else
    fHistoryState = nullptr;

#if EDITOR_MODE
  onEditorModeChanged();
#endif
}

#if EDITOR_MODE
///////////////////////////////////////////
// LCDDisplayView::onEditorModeChanged
///////////////////////////////////////////
void LCDDisplayView::onEditorModeChanged()
{
  if(fState && getEditorMode())
  {
    LCDData &lcdData = fState->fHistoryState->fLCDData;
    lcdData.fLeftChannel.fOn = true;
    lcdData.fRightChannel.fOn = true;

    auto dbLerp = Utils::Lerp<double>(0, -65.0, MAX_ARRAY_SIZE - 1, +0.5);

    for(int i = 0; i < MAX_ARRAY_SIZE; i++)
    {
      auto sample = i >= 10 && i < 13 ? 0.0: dbToSample<TSample>(dbLerp.computeY(i));
      lcdData.fLeftChannel.fSamples[i] = sample;
      lcdData.fRightChannel.fSamples[MAX_ARRAY_SIZE - i - 1] = sample;
    }

    for(int i = 100; i < 103; i++)
    {
      lcdData.fLeftChannel.fSamples[i] = 0.0;
      lcdData.fRightChannel.fSamples[i] = 0.0;
    }

    lcdData.fLeftChannel.fMaxLevelSinceReset = lcdData.fLeftChannel.fSamples[10];
    lcdData.fRightChannel.fMaxLevelSinceReset = lcdData.fRightChannel.fSamples[10];

    fState->fHistoryState->fMaxLevelInWindow =
      MaxLevel::computeMaxLevel(lcdData.fLeftChannel.computeInWindowMaxLevel(),
                                lcdData.fRightChannel.computeInWindowMaxLevel());
    fState->fHistoryState->fMaxLevelSinceReset =
      MaxLevel::computeMaxLevel(lcdData.fLeftChannel.computeSinceResetMaxLevel(),
                                lcdData.fRightChannel.computeSinceResetMaxLevel());

  }
}
#endif

LCDDisplayView::Creator __gLCDDisplayViewCreator("pongasoft::LCDDisplay", "pongasoft - LCD Display");

}
}
}