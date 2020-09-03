#include <vstgui4/vstgui/lib/controls/ccontrol.h>
#include <pongasoft/Utils/Clock/Clock.h>
#include <pongasoft/VST/AudioUtils.h>
#include "LCDDisplayView.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

const CColor MAX_LEVEL_FOR_SELECTION_LINES_COLOR = CColor{255,255,255,200};
const CColor MAX_LEVEL_SINCE_RESET_COLOR = CColor{255,0,0,220};
const CColor MAX_LEVEL_IN_WINDOW_COLOR = CColor{0,0,255,220};
const CColor MAX_LEVEL_FOR_SELECTION_COLOR = CColor{0,0,0,40};

///////////////////////////////////////////
// LCDDisplayState::LCDMessage::update
///////////////////////////////////////////
bool LCDDisplayState::LCDMessage::update(long iTime)
{
  if(isExpired(iTime))
    return true;

  // we bring now to "0" compare to fTime
  iTime -= fTime;
  auto startOfFade = fVisibleDuration;
  if(startOfFade <= iTime)
  {
    // we bring now to "0" compare to startOfFade
    iTime -= startOfFade;
    fColor.alpha = Utils::mapValueSPXY<long, uint8_t>(iTime, 0, fFadeDuration, 255, 0);
  }

  return false;
}

///////////////////////////////////////////
// LCDDisplayState::onTimer
///////////////////////////////////////////
void LCDDisplayState::onTimer(Timer * /* timer */)
{
  long now = Clock::getCurrentTimeMillis();

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

  if(fLCDSoftClippingLevelMessage == nullptr && fLCDZoomFactorXMessage == nullptr)
  {
    fTimer = nullptr;
  }
}

///////////////////////////////////////////
// LCDDisplayView::onTimer
///////////////////////////////////////////
void LCDDisplayView::onTimer(Timer *timer)
{
  LCDDisplayState::onTimer(timer);
  markDirty();
}

///////////////////////////////////////////
// LCDDisplayState::startTimer
///////////////////////////////////////////
void LCDDisplayState::startTimer()
{
  if(!fTimer)
  {
    fTimer = AutoReleaseTimer::create(this, UI_FRAME_RATE_MS);
  }
}

///////////////////////////////////////////
// LCDDisplayView::onParameterChange
///////////////////////////////////////////
void LCDDisplayView::onParameterChange(ParamID iParamID)
{
  if(iParamID == fLCDZoomFactorXParam.getParamID())
  {
    Steinberg::String text = "Zoom: ";
    text += fLCDZoomFactorXParam.toString();
    fLCDZoomFactorXMessage = std::make_unique<LCDMessage>(UTF8String(text), Clock::getCurrentTimeMillis());
    startTimer();
  }

  if(iParamID == fSoftClippingLevelParam.getParamID())
  {
    fLCDSoftClippingLevelMessage =
      std::make_unique<LCDMessage>(UTF8String(fSoftClippingLevelParam.toString()), Clock::getCurrentTimeMillis());
    startTimer();
  }

  CustomView::onParameterChange(iParamID);
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

  auto rdc = GUI::RelativeDrawContext{this, iContext};

  auto height = getViewSize().getHeight();
  auto width = getViewSize().getWidth();
  RelativeCoord left = 0;

  LCDData const &lcdData = fHistoryDataParam->fLCDData;

  bool leftChannelOn = lcdData.fLeftChannel.fOn;
  bool rightChannelOn = lcdData.fRightChannel.fOn;

  if(leftChannelOn || rightChannelOn)
  {

    MaxLevel maxLevelForSelection{-1, *fLCDInputXParameter};
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

      if(sample >= VST::Sample64SilentThreshold)
      {
        if(sample < MIN_AUDIO_SAMPLE) // a single pixel for -60dB to silent
          displayValue = 1;
        else
        {
          displayValue = toDisplayValue(sample, height);
        }

        top = Utils::clamp<RelativeCoord>(height - displayValue, -0.5, height);

        // color of the sample depends on its level
        CColor const &color = computeColor(*fSoftClippingLevelParameter, sample);

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

    if(*fMaxLevelInWindowMarker)
    {
      // on top of each other => make bigger
      CCoord size = 3.0;
      if(maxLevelInWindowPoint.x == maxLevelSinceResetPoint.x && *fMaxLevelSinceResetMarker)
      {
        size = 5.0;
      }
      drawMaxLevel(rdc, maxLevelInWindowPoint, size, MAX_LEVEL_IN_WINDOW_COLOR);
    }

    if(*fMaxLevelSinceResetMarker)
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
  auto softClippingDisplayValue = toDisplayValue(fSoftClippingLevelParameter.getValue().getValueInSample(), height);
  auto top = height - softClippingDisplayValue;

  // draw the soft clipping line
  rdc.drawLine(0, top, getWidth(), top, getSoftClippingLevelColor());


  // print the level
  if(fLCDSoftClippingLevelMessage)
  {
    StringDrawContext sdc{};
    sdc.addStyle(StringDrawContext::Style::kShadowText);
    sdc.fHorizTxtAlign = kLeftText;
    sdc.fTextInset = {2, 2};
    sdc.fFontColor = fLCDSoftClippingLevelMessage->fColor;
    sdc.fFont = fFont;
    sdc.fShadowColor = kBlackCColor;
    sdc.fShadowColor.alpha = fLCDSoftClippingLevelMessage->fColor.alpha;

    auto textTop = top + 3;
    rdc.drawString(fLCDSoftClippingLevelMessage->fText,
                   RelativeRect{0, textTop, static_cast<RelativeCoord>(MAX_ARRAY_SIZE), textTop + 20}, sdc);
  }

  if(fLCDZoomFactorXMessage)
  {
    StringDrawContext sdc{};
    sdc.fHorizTxtAlign = kCenterText;
    sdc.fTextInset = {2, 2};
    sdc.fFontColor = fLCDZoomFactorXMessage->fColor;
    sdc.fFont = fFont;

    rdc.drawString(fLCDZoomFactorXMessage->fText,
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
  if(*fLCDLiveViewParameter)
  {
    fLCDLiveViewParameter.setValue(false);
  }

  fLCDInputXEditor = fLCDInputXParameter.edit(computeLCDInputX(where));

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
  fMaxLevelSinceResetMarker = registerParam(fParams->fSinceResetMarkerParam);
  fMaxLevelInWindowMarker = registerParam(fParams->fInWindowMarkerParam);
  fLCDLiveViewParameter = registerParam(fParams->fLCDLiveViewParam);
  fLCDZoomFactorXParam = registerParam(fParams->fZoomFactorXParam);
  fSoftClippingLevelParam = registerParam(fParams->fSoftClippingLevelParam);
}

#if EDITOR_MODE

///////////////////////////////////////////
// LCDDisplayView::onEditorModeChanged
///////////////////////////////////////////
void LCDDisplayView::onEditorModeChanged()
{
  if(getEditorMode())
  {
    HistoryData historyData{};
    LCDData &lcdData = historyData.fLCDData;
    lcdData.fLeftChannel.fOn = true;
    lcdData.fRightChannel.fOn = true;

    auto dbLerp = Utils::mapRangeDPX<int>(0, MAX_ARRAY_SIZE - 1, -65.0, +0.5);

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

    historyData.computeMaxLevels();

    fHistoryDataParam.setValue(historyData);
  }
}

#endif

LCDDisplayView::Creator __gLCDDisplayViewCreator("VAC6V::LCDDisplay", "VAC6V - LCD Display");

}
}
}