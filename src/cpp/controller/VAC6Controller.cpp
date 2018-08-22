#include <vstgui4/vstgui/plugin-bindings/vst3editor.h>
#include <pongasoft/logging/loguru.hpp>
#include <pongasoft/VST/Debug/ParamDisplay.h>
#include <pongasoft/VST/Debug/ParamTable.h>
#include "VAC6Controller.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

//------------------------------------------------------------------------
// VAC6Controller::VAC6Controller
//------------------------------------------------------------------------
VAC6Controller::VAC6Controller() : GUIController("VAC6.uidesc"),
                                   fParameters{},
                                   fState{fParameters}
{
  DLOG_F(INFO, "VAC6Controller::VAC6Controller()");
}

//------------------------------------------------------------------------
// VAC6Controller::~VAC6Controller
//------------------------------------------------------------------------
VAC6Controller::~VAC6Controller()
{
  DLOG_F(INFO, "VAC6Controller::~VAC6Controller()");
}

//------------------------------------------------------------------------
// VAC6Controller::initialize
//------------------------------------------------------------------------
tresult VAC6Controller::initialize(FUnknown *context)
{
  tresult res = GUIController::initialize(context);

  //------------------------------------------------------------------------
  // In debug mode this code displays the order in which the GUI parameters
  // will be saved
  //------------------------------------------------------------------------
#ifndef NDEBUG
  if(res == kResultOk)
  {
    using Key = Debug::ParamDisplay::Key;
    DLOG_F(INFO, "GUI Save State - Version=%d --->\n%s",
           fParameters.getGUISaveStateOrder().fVersion,
           Debug::ParamTable::from(getGUIState(), true).keys({Key::kID, Key::kTitle}).full().toString().c_str());
  }
#endif

  return res;
}

}
}
}