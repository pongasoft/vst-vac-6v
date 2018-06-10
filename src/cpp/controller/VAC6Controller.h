#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui4/vstgui/plugin-bindings/vst3editor.h>
#include "VSTView.h"
#include "VAC6Views.h"


namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace Common;

/**
 * Represents the controller part of the plugin. Manages the UI.
 */
class VAC6Controller : public EditController, public VSTGUI::VST3EditorDelegate
{
public:
  VAC6Controller();

  ~VAC6Controller() override;


  /** Called at first after constructor */
  tresult PLUGIN_API initialize(FUnknown *context) override;

  /** Called at the end before destructor */
  tresult PLUGIN_API terminate() override;

  /** Create the view */
  IPlugView *PLUGIN_API createView(const char *name) override;

  /** Sets the component state (after setting the processor) or after restore */
  tresult PLUGIN_API setComponentState(IBStream *state) override;

  /** Restore the state (UI only!) (ex: after loading preset or project) */
  tresult PLUGIN_API setState(IBStream *state) override;

  /** Called to save the state (UI only!) (before saving a preset or project) */
  tresult PLUGIN_API getState(IBStream *state) override;

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
  // the name of the xml file (relative) which contains the ui description
  char const *const fXmlFile;

  // view for max level
  MaxLevelView fMaxLevelView;

  // view for LCD
  LCDView fLCDView;
};

}
}
}