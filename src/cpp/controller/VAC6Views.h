#pragma once

#include <vstgui4/vstgui/lib/controls/ctextlabel.h>
#include <pluginterfaces/vst/ivstmessage.h>

#include "VSTViewState.h"
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
class MaxLevelView : public VSTViewState<CTextLabel>
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


}
}
}