#pragma once

#include <pongasoft/VST/Messaging.h>
#include <pongasoft/Utils/Lerp.h>
#include <pongasoft/VST/GUI/Views/CustomView.h>
#include <pongasoft/VST/GUI/DrawContext.h>
#include <pongasoft/VST/GUI/GUIViewState.h>
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

/**
 * The view that will show the volume history as a graph
 */
class LCDDisplayView : public HistoryView
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
  void setFont(FontPtr iFont) { fFont = std::move(iFont); }

  // setState
  void setState(LCDDisplayState *iState);

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

#if EDITOR_MODE
public:
  // onEditorModeChanged
  void onEditorModeChanged() override;
#endif

protected:
  CColor fSoftClippingLevelColor{};
  FontPtr fFont{nullptr};

  // the state
  LCDDisplayState *fState{nullptr};

  GUIBooleanParamUPtr fMaxLevelSinceResetMarker{nullptr};
  GUIBooleanParamUPtr fMaxLevelInWindowMarker{nullptr};
  GUIBooleanParamUPtr fLCDLiveViewParameter{nullptr};

  GUIParamEditorUPtr<LCDInputXParamConverter> fLCDInputXEditor{nullptr};

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


constexpr long MESSAGE_VISIBLE_DURATION_MS = 2000;
constexpr long MESSAGE_FADE_DURATION_MS = 250;

/**
 * Keeps track of the lcd display sate whether the view is created or not */
class LCDDisplayState : public PluginGUIViewState<LCDDisplayView, VAC6Parameters>
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
    bool update(long iTime)
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
        auto lerp = Utils::Lerp<float>(0, 255, fFadeDuration, 0);
        float alpha = lerp.computeY(iTime);
        fColor.alpha = static_cast<uint8_t>(alpha);
      }

      return false;
    }

    long fVisibleDuration;
    long fFadeDuration;
    long fTime;
    CColor fColor;
    const UTF8String fText;
  };

public:
  // Constructor
  explicit LCDDisplayState(std::shared_ptr<HistoryState> iHistoryState) :
    fHistoryState{std::move(iHistoryState)},
    fLCDSoftClippingLevelMessage{nullptr},
    fLCDZoomFactorXMessage{nullptr}
  {};

  // Destructor
  ~LCDDisplayState() override = default;

  // onMessage (process message coming from the processor)
  void onMessage(Message const &message);

  // afterAssign
  void afterAssign() override;

  void registerParameters() override;

  void onParameterChange(ParamID iParamID, ParamValue iNormalizedValue) override;

protected:
  // onSoftClippingLevelChange
  void onSoftClippingLevelChange();

  // onZoomFactorXChange
  void onZoomFactorXChange();

private:
  friend class LCDDisplayView;

  std::shared_ptr<HistoryState> fHistoryState;
  std::unique_ptr<LCDMessage> fLCDSoftClippingLevelMessage;
  std::unique_ptr<LCDMessage> fLCDZoomFactorXMessage;

  GUIParamUPtr<SoftClippingLevelParamConverter> fSoftClippingLevelParam{nullptr};
  GUIParamUPtr<LCDZoomFactorXParamConverter> fLCDZoomFactorXParam{nullptr};
};

}
}
}
