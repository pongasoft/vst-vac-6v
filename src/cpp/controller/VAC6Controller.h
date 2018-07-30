#pragma once

#include <vstgui4/vstgui/plugin-bindings/vst3editor.h>
#include <pongasoft/VST/GUI/GUIController.h>
#include "HistoryView.h"
#include "../VAC6Plugin.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

/**
 * Represents the controller part of the plugin. Manages the UI.
 */
class VAC6Controller : public GUI::GUIController, public VSTGUI::VST3EditorDelegate
{
public:
  // Constructor
  VAC6Controller();

  // Destructor
  ~VAC6Controller() override;

  // getGUIState
  GUIState *getGUIState() override { return &fState; }

 /** From VST3EditorDelegate to be able to get a handle to some of the views */
  CView *verifyView(CView *view,
                    const UIAttributes &attributes,
                    const IUIDescription *description,
                    VST3Editor *editor) override;

  /** From ComponentBase to receive messages */
  tresult PLUGIN_API notify(IMessage *message) SMTG_OVERRIDE;

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

  // the history state (shared by fMaxLevelState & fLCDDisplayState)
  std::shared_ptr<HistoryState> fHistoryState;
};

}
}
}