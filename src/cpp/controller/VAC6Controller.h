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
  // Factory method
  static FUnknown *createInstance(void * /*context*/) { return (IEditController *) new VAC6Controller(); }

public:
  // Constructor
  VAC6Controller();

  // Destructor
  ~VAC6Controller() override;

  // getGUIState
  GUIState *getGUIState() override { return &fState; }


protected:
  tresult initialize(FUnknown *context) override;

private:
  VAC6Parameters fParameters;
  VAC6GUIState fState;
};

}
}
}