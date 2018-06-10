#pragma once

#include <vstgui4/vstgui/lib/controls/ctextlabel.h>
#include <pluginterfaces/vst/ivstmessage.h>
#include "VSTView.h"
#include "../VAC6Constants.h"
#include "../Messaging.h"
#include "../VAC6Model.h"
#include "CustomDisplayView.h"

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
  MaxLevelView() : fMaxLevel{0, kStateOk} {}

  void setMaxLevel(MaxLevel const &maxLevel);

  void afterAssign() override { updateView(); };

  void onMessage(Message const &message);

private:
  void updateView() const;

  MaxLevel fMaxLevel;
};

/**
 * Handles the LCD screen that displays the graph */
class LCDView : public VSTView<CustomDisplayView>
{
public:
  LCDView() : fLCDData{} {};

  void onMessage(Message const &message);

  void afterAssign() override { updateView(); };

private:
  void updateView() const;

  LCDData fLCDData;
};

}
}
}