#include <vstgui4/vstgui/plugin-bindings/vst3editor.h>
#include <pongasoft/logging/loguru.hpp>
#include "VAC6Controller.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// VAC6Controller::VAC6Controller
///////////////////////////////////////////
VAC6Controller::VAC6Controller() : EditController(),
                                   fXmlFile("VAC6.uidesc"),
                                   fPluginParameters{},
                                   fGUIParameters{HostParameters(this), fPluginParameters},
                                   fHistoryState{std::make_shared<HistoryState>()},
                                   fMaxLevelSinceResetState{MaxLevelState::Type::kSinceReset, fHistoryState},
                                   fMaxLevelInWindowState{MaxLevelState::Type::kInWindow, fHistoryState},
                                   fMaxLevelForSelectionState{MaxLevelState::Type::kForSelection, fHistoryState},
                                   fLCDDisplayState{fHistoryState}
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

///////////////////////////////////////////
// VAC6Controller::initialize
///////////////////////////////////////////
tresult VAC6Controller::initialize(FUnknown *context)
{
  DLOG_F(INFO, "VAC6Controller::initialize()");

  tresult result = EditController::initialize(context);
  if(result != kResultOk)
  {
    return result;
  }

  // making sure that the knob mode is linear
  CFrame::kDefaultKnobMode = CKnobMode::kLinearMode;
  setKnobMode(CKnobMode::kLinearMode);

  fGUIParameters.registerVstParameters(parameters);

  fViewFactory = new CustomUIViewFactory(fGUIParameters);

  return result;
}

///////////////////////////////////////////
// VAC6Controller::terminate
///////////////////////////////////////////
tresult VAC6Controller::terminate()
{
  delete fViewFactory;

  return EditController::terminate();
}

///////////////////////////////////////////
// VAC6Controller::createView
///////////////////////////////////////////
IPlugView *VAC6Controller::createView(const char *name)
{
  if(name && strcmp(name, ViewType::kEditor) == 0)
  {
    UIDescription *uiDescription = new UIDescription(fXmlFile,fViewFactory);
    return new VSTGUI::VST3Editor(uiDescription, this, "view", fXmlFile);
  }
  return nullptr;
}

///////////////////////////////////////////
// VAC6Controller::verifyView
///////////////////////////////////////////
CView *VAC6Controller::verifyView(CView *view,
                                  const UIAttributes &attributes,
                                  const IUIDescription * /*description*/,
                                  VST3Editor * /*editor*/)
{
  auto customView = dynamic_cast<CustomView *>(view);
  if(customView != nullptr)
  {
    switch(customView->getCustomViewTag())
    {
      case EVAC6CustomViewTag::kMaxLevelSinceReset:
        fMaxLevelSinceResetState.assign(dynamic_cast<MaxLevelView *>(customView));
        break;

      case EVAC6CustomViewTag::kMaxLevelInWindow:
        fMaxLevelInWindowState.assign(dynamic_cast<MaxLevelView *>(customView));
        break;

      case EVAC6CustomViewTag::kMaxLevelForSelection:
        fMaxLevelForSelectionState.assign(dynamic_cast<MaxLevelView *>(customView));
        break;

      case EVAC6CustomViewTag::kLCD:
        fLCDDisplayState.assign(dynamic_cast<LCDDisplayView *>(customView));
        break;

      default:
        // ignoring other
        break;

    }
  }

  return view;
}

///////////////////////////////////////////
// VAC6Controller::setComponentState
///////////////////////////////////////////
tresult VAC6Controller::setComponentState(IBStream *state)
{
  // DLOG_F(INFO, "VAC6Controller::setComponentState");

  // we receive the current state of the component (processor part)
  if(state == nullptr)
    return kResultFalse;

  // using helper to read the stream
  IBStreamer streamer(state, kLittleEndian);
  return fGUIParameters.readRTState(streamer);
}

///////////////////////////////////
// VAC6Controller::setState
///////////////////////////////////
tresult VAC6Controller::setState(IBStream *state)
{
  if(state == nullptr)
    return kResultFalse;

  // DLOG_F(INFO, "VAC6Controller::setState()");

  IBStreamer streamer(state, kLittleEndian);
  return fGUIParameters.readGUIState(streamer);
}

///////////////////////////////////
// VAC6Controller::getState
///////////////////////////////////
tresult VAC6Controller::getState(IBStream *state)
{
  if(state == nullptr)
    return kResultFalse;

  // DLOG_F(INFO, "VAC6Controller::getState()");

  IBStreamer streamer(state, kLittleEndian);
  return fGUIParameters.writeGUIState(streamer);
}

///////////////////////////////////
// VAC6Controller::notify
///////////////////////////////////
tresult VAC6Controller::notify(IMessage *message)
{
  if(!message)
    return kInvalidArgument;

  Message m{message};

  switch(static_cast<EVAC6MessageID>(m.getMessageID()))
  {
    case kLCDData_MID:
    {
      fHistoryState->onMessage(m);
      fMaxLevelSinceResetState.onMessage(m);
      fMaxLevelInWindowState.onMessage(m);
      fMaxLevelForSelectionState.onMessage(m);
      fLCDDisplayState.onMessage(m);
      break;
    }

    default:
      DLOG_F(WARNING, "VAC6Controller::notify / unhandled message id %d", m.getMessageID());
      return kResultFalse;
  }

  return kResultOk;
}

///////////////////////////////////
// VAC6Controller::setParamNormalized
///////////////////////////////////
template<typename ParamConverter>
void VAC6Controller::setParamNormalized(ParamID iParamID, IBStreamer &iStreamer, typename ParamConverter::ParamType const &iDefaultValue)
{
  double value;
  if(!iStreamer.readDouble(value))
    value = ParamConverter::normalize(iDefaultValue);
  EditController::setParamNormalized(iParamID, value);
}

}
}
}