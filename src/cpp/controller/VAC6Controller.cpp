#include <vstgui4/vstgui/plugin-bindings/vst3editor.h>
#include <pongasoft/logging/loguru.hpp>
#include "VAC6Controller.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// VAC6Controller::VAC6Controller
///////////////////////////////////////////
VAC6Controller::VAC6Controller() : GUIController("VAC6.uidesc"),
                                   fParameters{},
                                   fState{fParameters}
{
  DLOG_F(INFO, "VAC6Controller::VAC6Controller()");
}

///////////////////////////////////////////
// VAC6Controller::~VAC6Controller
///////////////////////////////////////////
VAC6Controller::~VAC6Controller()
{
  DLOG_F(INFO, "VAC6Controller::~VAC6Controller()");
}

}
}
}