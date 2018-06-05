#pragma once

#include <vstgui4/vstgui/lib/controls/ctextlabel.h>
#include <pluginterfaces/vst/ivstmessage.h>
#include "VSTView.h"
#include "../VAC6Constants.h"
#include "../Messaging.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace VSTGUI;
using namespace Common;
using namespace Steinberg::Vst;

/**
 * Handles the max level text label */
class MaxLevelView : public VSTView<CTextLabel>
{
public:
  MaxLevelView();

  void setMaxLevel(MaxLevel const &maxLevel);

  void afterAssign() override;

  void onMessage(Message const &message);

private:
  void updateView() const;

  MaxLevel fMaxLevel;
};

}
}
}