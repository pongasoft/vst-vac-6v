#pragma once

#include <pongasoft/VST/GUI/GUIController.h>
#include "HistoryView.h"
#include "../VAC6Plugin.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

/**
 * Represents the controller part of the plugin. Manages the UI.
 */
class VAC6Controller : public GUI::GUIController
{
public:
  // Constructor
  VAC6Controller();

  // Destructor
  ~VAC6Controller() override;

  // getGUIState
  GUIState *getGUIState() override { return &fState; }

  //--- ---------------------------------------------------------------------
  // create function required for Plug-in factory,
  // it will be called to create new instances of this controller
  //--- ---------------------------------------------------------------------
  static FUnknown *createInstance(void * /*context*/)
  {
    return (IEditController *) new VAC6Controller();
  }

private:
  VAC6Parameters fParameters;
  VAC6GUIState fState;
};

}
}
}