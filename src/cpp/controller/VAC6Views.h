#pragma once

#include <vstgui4/vstgui/lib/controls/ctextlabel.h>
#include <pluginterfaces/vst/ivstmessage.h>

#include "VSTView.h"
#include "../VAC6Constants.h"
#include "../Messaging.h"
#include "../VAC6Model.h"
#include "CustomDisplayView.h"
#include "DrawContext.h"
#include "../Utils.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace VSTGUI;
using namespace Common;
using namespace Steinberg::Vst;
using namespace GUI;

/**
 * Handles the max level text label */
class MaxLevelView : public VSTView<CTextLabel>
{
public:
  MaxLevelView() : fMaxLevel{}
  {}

  void setMaxLevel(MaxLevel const &maxLevel);

  void afterAssign() override
  {
    updateView();
  };

  void onMessage(Message const &message);

private:
  void updateView() const;

  MaxLevel fMaxLevel;
};

constexpr long MESSAGE_VISIBLE_DURATION_MS = 2000;
constexpr long MESSAGE_FADE_DURATION_MS = 250;

/**
 * Handles the LCD screen that displays the graph */
class LCDView : public VSTView<CustomDisplayView>
{
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

    bool isExpired(long iTime) const
    {
      return fTime + fVisibleDuration + fFadeDuration <= iTime;
    }

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
  LCDView() : fLCDData{}, fLCDSoftClipingLevelMessage{nullptr}, fLCDZoomFactorXMessage{nullptr}
  {};

  ~LCDView() override
  {
    delete fLCDZoomFactorXMessage;
    delete fLCDSoftClipingLevelMessage;
  }

  void onMessage(Message const &message);

  void afterAssign() override;

  void beforeUnassign() override;

private:
  void updateView() const;

  void draw(CDrawContext *iContext) const;

  LCDData fLCDData;
  LCDMessage *fLCDSoftClipingLevelMessage;
  LCDMessage *fLCDZoomFactorXMessage;
};

}
}
}