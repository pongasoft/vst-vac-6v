#include <vstgui4/vstgui/plugin-bindings/vst3editor.h>
#include <base/source/fstreamer.h>
#include "logging/loguru.hpp"
#include "VAC6Controller.h"
#include "VAC6CIDs.h"

namespace pongasoft {
namespace VST {

///////////////////////////////////////////
// VAC6Controller::VAC6Controller
///////////////////////////////////////////
VAC6Controller::VAC6Controller() : EditController(),
  fXmlFile("VAC6.uidesc")
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

  // Max Level Value
  parameters.addParameter(STR16 ("Max Level Value"), // title
                          nullptr, // units
                          0, // stepCount (continuous)
                          0, // defaultNormalizedValue
                          ParameterInfo::kIsReadOnly, // flags
                          EVAC6ParamID ::kMaxLevelValue); // tag

  // Max Level State (0 = ok, 1 = soft clipping, 2 = hard clipping) for Max Level Value
  parameters.addParameter(STR16 ("Max Level State"), // title
                          nullptr, // units
                          2, // stepCount = 2 => 3 values
                          0, // defaultNormalizedValue
                          ParameterInfo::kIsReadOnly, // flags
                          EVAC6ParamID ::kMaxLevelState); // tag

  return result;
}

///////////////////////////////////////////
// VAC6Controller::terminate
///////////////////////////////////////////
tresult VAC6Controller::terminate()
{
  return EditController::terminate();
}

///////////////////////////////////////////
// VAC6Controller::createView
///////////////////////////////////////////
IPlugView *VAC6Controller::createView(const char *name)
{
  if(name && strcmp(name, ViewType::kEditor) == 0)
  {
    return new VSTGUI::VST3Editor(this, "view", fXmlFile);
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
  DLOG_F(INFO, "VAC6Controller::verifyView()");

  auto te = dynamic_cast<CTextEdit *>(view);
  if(te != nullptr)
  {
    switch(te->getTag())
    {
      // TODO

      default:
        // nothing to do in this case
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
  // we receive the current state of the component (processor part)
  if(state == nullptr)
    return kResultFalse;

  // using helper to read the stream
  IBStreamer streamer(state, kLittleEndian);

  // TODO

  DLOG_F(INFO, "VAC6Controller::setComponentState => ");

  return kResultOk;
}

///////////////////////////////////
// VAC6Controller::setState
///////////////////////////////////
tresult VAC6Controller::setState(IBStream *state)
{
  if(state == nullptr)
    return kResultFalse;

  DLOG_F(INFO, "VAC6Controller::setState()");

  IBStreamer streamer(state, kLittleEndian);

  // TODO

  return kResultOk;
}

///////////////////////////////////
// VAC6Controller::getState
///////////////////////////////////
tresult VAC6Controller::getState(IBStream *state)
{
  if(state == nullptr)
    return kResultFalse;

  DLOG_F(INFO, "VAC6Controller::getState()");

  IBStreamer streamer(state, kLittleEndian);

  // TODO

  return kResultOk;
}


}
}