#pragma once

#include <vstgui4/vstgui/lib/controls/ctextlabel.h>
#include "VSTView.h"
#include "../VAC6Constants.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace VSTGUI;
using namespace Common;

class MaxLevelView : public VSTView<CTextLabel>
{
public:
  MaxLevelView();

  void setMaxLevel(MaxLevel const& maxLevel);

  void afterAssign() override;

private:
  void updateView() const;

  MaxLevel fMaxLevel;
};

}
}
}