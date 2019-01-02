#pragma once

#include <pongasoft/VST/Messaging.h>
#include <pongasoft/VST/Timer.h>
#include <pongasoft/VST/GUI/Views/CustomView.h>
#include <pongasoft/VST/GUI/DrawContext.h>
#include <memory>
#include "../VAC6Model.h"
#include "HistoryView.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace VSTGUI;
using namespace Common;
using namespace GUI;

class LCDDisplayState;

constexpr long MESSAGE_VISIBLE_DURATION_MS = 2000;
constexpr long MESSAGE_FADE_DURATION_MS = 250;

/**
 * Keeps track of the lcd display sate (messages) */
class LCDDisplayState : public ITimerCallback
{
  /**
   * A message to display on the LCD Screen (with auto fade)
   */
  struct LCDMessage
  {
    LCDMessage(UTF8String iText, long iTime,
               CColor const &iColor = kWhiteCColor,
               long iVisibleDuration = MESSAGE_VISIBLE_DURATION_MS,
               long iFadeDuration = MESSAGE_FADE_DURATION_MS) :
      fText(std::move(iText)),
      fTime{iTime},
      fColor{iColor},
      fVisibleDuration{iVisibleDuration},
      fFadeDuration{iFadeDuration}
    {
    }

    // isExpired
    bool isExpired(long iTime) const
    {
      return fTime + fVisibleDuration + fFadeDuration <= iTime;
    }

    // update
    bool update(long iTime);

    long fVisibleDuration;
    long fFadeDuration;
    long fTime;
    CColor fColor;
    const UTF8String fText;
  };

protected:
  void onTimer(Timer *timer) override;

  void startTimer();

private:
  friend class LCDDisplayView;

  std::unique_ptr<LCDMessage> fLCDSoftClippingLevelMessage;
  std::unique_ptr<LCDMessage> fLCDZoomFactorXMessage;

  std::unique_ptr<AutoReleaseTimer> fTimer;
};

/**
 * The view that will show the volume history as a graph
 */
class LCDDisplayView : public HistoryView, public LCDDisplayState
{
public:
  // Constructor
  explicit LCDDisplayView(const CRect &size);

  LCDDisplayView(const LCDDisplayView &c) = delete;

  // get/setSoftClippingLevelColor
  const CColor &getSoftClippingLevelColor() const { return fSoftClippingLevelColor; }
  void setSoftClippingLevelColor(const CColor &iSoftClippingLevelColor) { fSoftClippingLevelColor = iSoftClippingLevelColor; }

  // get/setFont
  FontPtr getFont() const { return fFont; }
  void setFont(FontPtr iFont) { fFont = iFont; }

public:

  // draw => does the actual drawing job
  void draw(CDrawContext *iContext) override;

  // registerParameters
  void registerParameters() override;

  // onMouseDown
  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;

  // onMouseMoved
  CMouseEventResult onMouseMoved(CPoint &where, const CButtonState &buttons) override;

  // onMouseUp
  CMouseEventResult onMouseUp(CPoint &where, const CButtonState &buttons) override;

  // onMouseCancel
  CMouseEventResult onMouseCancel() override;

  CLASS_METHODS_NOCOPY(LCDDisplayView, HistoryView)

protected:
  // computeLCDInputX
  int computeLCDInputX(CPoint &where) const;

  // drawMaxLevel
  void drawMaxLevel(GUI::RelativeDrawContext &iContext, RelativePoint const &iPoint, CCoord iHalfSize, CColor const &iColor);

  // drawMaxLevelNoCheck
  void drawMaxLevelNoCheck(GUI::RelativeDrawContext &iContext, RelativePoint const &iPoint, CCoord iHalfSize, CColor const &iColor);

  // onParameterChange
  void onParameterChange(ParamID iParamID) override;

  // onTimer
  void onTimer(Timer *timer) override;

#if EDITOR_MODE
public:
  // onEditorModeChanged
  void onEditorModeChanged() override;
#endif

protected:
  CColor fSoftClippingLevelColor{};
  FontSPtr fFont{nullptr};

  GUIVstBooleanParam fMaxLevelSinceResetMarker{nullptr};
  GUIVstBooleanParam fMaxLevelInWindowMarker{nullptr};
  GUIVstBooleanParam fLCDLiveViewParameter{nullptr};

  GUIVstParam<SoftClippingLevel> fSoftClippingLevelParam{nullptr};
  GUIVstParam<Percent> fLCDZoomFactorXParam{nullptr};

  GUIVstParamEditor<int> fLCDInputXEditor{nullptr};

public:
  class Creator : public CustomViewCreator<LCDDisplayView, HistoryView>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerColorAttribute("soft-clipping-level-color",
                             &LCDDisplayView::getSoftClippingLevelColor,
                             &LCDDisplayView::setSoftClippingLevelColor);
      registerFontAttribute("font",
                            &LCDDisplayView::getFont,
                            &LCDDisplayView::setFont);
    }
  };

};

}
}
}
