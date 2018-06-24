#include "VAC6Views.h"
#include "../AudioUtils.h"
#include "../Utils.h"
#include "../Clock.h"

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


}
}
}