#include "GUIController.h"
#include <vstgui4/vstgui/plugin-bindings/vst3editor.h>
#include <pongasoft/VST/GUI/Views/JambaViews.h>

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace Views;
using namespace VSTGUI;

//------------------------------------------------------------------------
// GUIController::GUIController
//------------------------------------------------------------------------
GUIController::GUIController(const char *iXmlFileName) :
  EditController(),
  fXmlFileName{iXmlFileName}
{
  Views::JambaViews::instance();
}

//------------------------------------------------------------------------
// GUIController::~GUIController
//------------------------------------------------------------------------
GUIController::~GUIController()
{
  delete fViewFactory;
}

//------------------------------------------------------------------------
// GUIController::initialize
//------------------------------------------------------------------------
tresult GUIController::initialize(FUnknown *context)
{
#ifdef JAMBA_DEBUG_LOGGING
  DLOG_F(INFO, "GUIController::initialize()");
#endif

  tresult result = EditController::initialize(context);
  if(result != kResultOk)
  {
    return result;
  }

  // vst parameters
  VstParametersSPtr vstParameters = std::make_shared<VstParameters>(this);

  // initializing the state
  auto guiState = getGUIState();

  guiState->init(vstParameters);

  // making sure that the knob mode is set to the default specified
  CFrame::kDefaultKnobMode = fDefaultKnobMode;
  setKnobMode(fDefaultKnobMode);

  guiState->getPluginParameters().registerVstParameters(parameters);

  fViewFactory = new CustomUIViewFactory(guiState);

  for(const auto &viewState : fViewStates)
  {
    viewState->initState(guiState);
    viewState->registerParameters();
  }

  return result;
}

//------------------------------------------------------------------------
// GUIController::terminate
//------------------------------------------------------------------------
tresult GUIController::terminate()
{
#ifdef JAMBA_DEBUG_LOGGING
  DLOG_F(INFO, "GUIController::terminate()");
#endif

  tresult res = EditController::terminate();

  delete fViewFactory;
  fViewFactory = nullptr;

  return res;
}

//------------------------------------------------------------------------
// GUIController::createView
//------------------------------------------------------------------------
IPlugView *GUIController::createView(const char *name)
{
  if(name && strcmp(name, ViewType::kEditor) == 0)
  {
    UIDescription *uiDescription = new UIDescription(fXmlFileName, fViewFactory);
    return new VSTGUI::VST3Editor(uiDescription, this, "view", fXmlFileName);
  }
  return nullptr;
}

//------------------------------------------------------------------------
// VAC6Controller::setComponentState
//------------------------------------------------------------------------
tresult GUIController::setComponentState(IBStream *state)
{
  // we receive the current state of the component (processor part)
  if(state == nullptr)
    return kResultFalse;

  // using helper to read the stream
  IBStreamer streamer(state, kLittleEndian);
  return getGUIState()->readRTState(streamer);
}

//------------------------------------------------------------------------
// VAC6Controller::setState
//------------------------------------------------------------------------
tresult GUIController::setState(IBStream *state)
{
  if(state == nullptr)
    return kResultFalse;

  IBStreamer streamer(state, kLittleEndian);
  return getGUIState()->readGUIState(streamer);
}

//------------------------------------------------------------------------
// VAC6Controller::getState
//------------------------------------------------------------------------
tresult GUIController::getState(IBStream *state)
{
  if(state == nullptr)
    return kResultFalse;

  IBStreamer streamer(state, kLittleEndian);
  return getGUIState()->writeGUIState(streamer);
}

//------------------------------------------------------------------------
// VAC6Controller::getState
//------------------------------------------------------------------------
void GUIController::registerViewState(std::shared_ptr<GUIViewState> iViewState)
{
  if(std::find(fViewStates.cbegin(), fViewStates.cend(), iViewState) != fViewStates.cend())
  {
    DLOG_F(WARNING, "Registering same GUIViewState multiple times (ignored");
  }
  else
  {
    fViewStates.emplace_back(std::move(iViewState));
  }
}

}
}
}