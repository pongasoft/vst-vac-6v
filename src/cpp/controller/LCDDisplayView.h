#pragma once

#include "CustomDisplayView.h"
#include "VSTViewState.h"
#include "DrawContext.h"
#include "../VAC6Model.h"
#include "../Messaging.h"
#include "../Utils.h"
#include "VAC6Views.h"
#include "VSTParameters.h"
#include "../VAC6CIDs.h"
#include <memory>
#include <utility>

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

  LCDDisplayView(const LCDDisplayView &c) = default;

  // getSoftClippingLevelColor
  const CColor &getSoftClippingLevelColor() const
  {
    return fSoftClippingLevelColor;
  }

  // setSoftClippingLevelColor
  void setSoftClippingLevelColor(const CColor &iSoftClippingLevelColor)
  {
    fSoftClippingLevelColor = iSoftClippingLevelColor;
  }

  // setState
  void setState(LCDDisplayState *iState)
  {
    fState = iState;
  }

  void initParameters(std::shared_ptr<VSTParameters> iParameters)
  {
    fParameters = std::move(iParameters);
  }

public:

  // draw => does the actual drawing job
  void draw(CDrawContext *iContext) override;

  CMouseEventResult onMouseDown(CPoint &where, const CButtonState &buttons) override;

  CLASS_METHODS(LCDDisplayView, HistoryView)

protected:
  CColor fSoftClippingLevelColor{};

  // the state
  LCDDisplayState *fState{nullptr};

  // Access to parameters
  std::shared_ptr<VSTParameters> fParameters{nullptr};
};

constexpr long MESSAGE_VISIBLE_DURATION_MS = 2000;
constexpr long MESSAGE_FADE_DURATION_MS = 250;

/**
 * Keeps track of the lcd display sate whether the view is created or not */
class LCDDisplayState : public VSTViewState<LCDDisplayView>
{
  /**
   * A message to display on the LCD Screen (with auto fade)
   */
  struct LCDMessage
  {
    LCDMessage(UTF8String iText, long iTime,
               CColor const &iColor = WHITE_COLOR,
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
        float alpha = lerp.compute(iTime);
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
  LCDDisplayState() : fLCDData{}, fLCDSoftClipingLevelMessage{nullptr}, fLCDZoomFactorXMessage{nullptr}
  {};

  // Destructor
  ~LCDDisplayState() override
  {
    delete fLCDZoomFactorXMessage;
    delete fLCDSoftClipingLevelMessage;
  }

  // onMessage (process message coming from the processor)
  void onMessage(Message const &message);

  // afterAssign
  void afterAssign() override;

  // beforeUnassign
  void beforeUnassign() override;

private:
  friend class LCDDisplayView;

  void updateView() const;

  LCDData fLCDData;
  LCDMessage *fLCDSoftClipingLevelMessage;
  LCDMessage *fLCDZoomFactorXMessage;
};

/**
 * The factory for LCDDisplayView
 */
class LCDDisplayViewCreator : public CustomViewCreator<LCDDisplayView>
{
public:
  explicit LCDDisplayViewCreator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
    CustomViewCreator(iViewName, iDisplayName)
  {
    registerAttributes(HistoryViewCreator());
    registerColorAttribute("soft-clipping-level-color",
                           &LCDDisplayView::getSoftClippingLevelColor,
                           &LCDDisplayView::setSoftClippingLevelColor);
  }
};

}
}
}
