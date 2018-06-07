#include <vstgui4/vstgui/plugin-bindings/vst3editor.h>
#include <base/source/fstreamer.h>
#include "../logging/loguru.hpp"
#include "VAC6Controller.h"
#include "../VAC6CIDs.h"
#include "../VAC6Model.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// VAC6Controller::VAC6Controller
///////////////////////////////////////////
VAC6Controller::VAC6Controller() : EditController(),
  fXmlFile("VAC6.uidesc"),
  fMaxLevelView{}
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

  // the knob that changes the soft clipping level
  parameters.addParameter(STR16 ("Soft Clipping Level"), // title
                          nullptr, // units
                          0, // stepCount => continuous
                          0.75, // defaultNormalizedValue
                          Vst::ParameterInfo::kCanAutomate, // flags
                          EVAC6ParamID::kSoftClippingLevel, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("Sft Clp Lvl")); // shortTitle

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

  auto control = dynamic_cast<CControl *>(view);
  if(control != nullptr)
  {
    switch(control->getTag())
    {
      case kMaxLevelValue:
        fMaxLevelView.assign(dynamic_cast<CTextLabel *>(control));
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
  // we receive the current state of the component (processor part)
  if(state == nullptr)
    return kResultFalse;

  // using helper to read the stream
  IBStreamer streamer(state, kLittleEndian);

  // EVAC6ParamID::kSoftClippingLevel
  double savedParamSoftLevelClipping = 0.f;
  if(!streamer.readDouble(savedParamSoftLevelClipping))
    return kResultFalse;
  setParamNormalized(EVAC6ParamID::kSoftClippingLevel, SoftClippingLevel{savedParamSoftLevelClipping}.getNormalizedParam());

  DLOG_F(INFO, "VAC6Controller::setComponentState => kSoftClippingLevel=%f",
         savedParamSoftLevelClipping);

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

  return kResultOk;
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
    case kMaxLevel_MID:
    {
      fMaxLevelView.onMessage(m);
      break;
    }

    default:
      DLOG_F(WARNING, "VAC6Controller::notify / unhandled message id %d", m.getMessageID());
      return kResultFalse;
  }

  return kResultOk;
}

}
}
}