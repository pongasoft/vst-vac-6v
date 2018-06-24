#include <vstgui4/vstgui/plugin-bindings/vst3editor.h>
#include <base/source/fstreamer.h>
#include <pluginterfaces/base/ustring.h>
#include "../logging/loguru.hpp"
#include "VAC6Controller.h"
#include "../VAC6CIDs.h"
#include "../Parameter.h"
#include "MaxLevelView.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// VAC6Controller::VAC6Controller
///////////////////////////////////////////
VAC6Controller::VAC6Controller() : EditController(),
                                   fXmlFile("VAC6.uidesc"),
                                   fMaxLevelState{},
                                   fLCDDisplayState{}
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
                          SoftClippingLevel{DEFAULT_SOFT_CLIPPING_LEVEL}.getNormalizedParam(), // defaultNormalizedValue
                          0, // flags
                          EVAC6ParamID::kSoftClippingLevel, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("Sft Clp Lvl")); // shortTitle

  // the momentary button that resets the max level
  parameters.addParameter(STR16 ("Max Level Reset"), // title
                          nullptr, // units
                          1, // stepCount => 1 means toggle
                          normalizeBoolValue(false), // defaultNormalizedValue
                          0, // flags
                          EVAC6ParamID::kMaxLevelReset, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("Max Lvl Rst")); // shortTitle

  // An option menu to change the auto reset (from OFF to 10s)
  auto maxLevelAutoResetParam = new StringListParameter(USTRING("Max Level Auto Reset"),
                                                        EVAC6ParamID::kMaxLevelAutoReset);
  maxLevelAutoResetParam->appendString(USTRING("OFF"));
  maxLevelAutoResetParam->appendString(USTRING("1s"));
  maxLevelAutoResetParam->appendString(USTRING("2s"));
  maxLevelAutoResetParam->appendString(USTRING("3s"));
  maxLevelAutoResetParam->appendString(USTRING("4s"));
  maxLevelAutoResetParam->appendString(USTRING("5s"));
  maxLevelAutoResetParam->appendString(USTRING("6s"));
  maxLevelAutoResetParam->appendString(USTRING("7s"));
  maxLevelAutoResetParam->appendString(USTRING("8s"));
  maxLevelAutoResetParam->appendString(USTRING("9s"));
  maxLevelAutoResetParam->appendString(USTRING("10s"));
  parameters.addParameter(maxLevelAutoResetParam);

  // the momentary button that resets the max level
  parameters.addParameter(STR16 ("Zoom Level"), // title
                          nullptr, // units
                          0, // stepCount => continuous
                          DEFAULT_ZOOM_FACTOR_X, // defaultNormalizedValue
                          0, // flags
                          EVAC6ParamID::kLCDZoomFactorX, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("Zoom Lvl")); // shortTitle

  // on/off toggle to show/hide left channel
  parameters.addParameter(STR16 ("Left Channel"), // title
                          nullptr, // units
                          1, // stepCount => 1 means toggle
                          normalizeBoolValue(true), // defaultNormalizedValue
                          0, // flags
                          EVAC6ParamID::kLCDLeftChannel, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("L Chan")); // shortTitle

  // on/off toggle to show/hide right channel
  parameters.addParameter(STR16 ("Right Channel"), // title
                          nullptr, // units
                          1, // stepCount => 1 means toggle
                          normalizeBoolValue(true), // defaultNormalizedValue
                          0, // flags
                          EVAC6ParamID::kLCDRightChannel, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("R Chan")); // shortTitle

  // on/off toggle to show live view/pause
  parameters.addParameter(STR16 ("Live"), // title
                          nullptr, // units
                          1, // stepCount => 1 means toggle
                          normalizeBoolValue(true), // defaultNormalizedValue
                          0, // flags
                          EVAC6ParamID::kLCDLiveView, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("Live")); // shortTitle

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
  auto control = dynamic_cast<CControl *>(view);
  if(control != nullptr)
  {
    switch(control->getTag())
    {
      case kMaxLevelValue:
        fMaxLevelState.assign(dynamic_cast<MaxLevelView *>(control));
        break;

      case kLCD:
        fLCDDisplayState.assign(dynamic_cast<LCDDisplayView *>(control));
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
  DLOG_F(INFO, "VAC6Controller::setComponentState");

  // we receive the current state of the component (processor part)
  if(state == nullptr)
    return kResultFalse;

  // using helper to read the stream
  IBStreamer streamer(state, kLittleEndian);

  // EVAC6ParamID::kSoftClippingLevel
  double savedParamSoftLevelClipping = 0.f;
  if(!streamer.readDouble(savedParamSoftLevelClipping))
    savedParamSoftLevelClipping = DEFAULT_SOFT_CLIPPING_LEVEL;
  setParamNormalized(EVAC6ParamID::kSoftClippingLevel,
                     SoftClippingLevel{savedParamSoftLevelClipping}.getNormalizedParam());

  // EVAC6ParamID::kLCDZoomFactorX
  double savedParamZoomFactorX = 0.f;
  if(!streamer.readDouble(savedParamZoomFactorX))
    savedParamZoomFactorX = DEFAULT_ZOOM_FACTOR_X;
  setParamNormalized(EVAC6ParamID::kLCDZoomFactorX, savedParamZoomFactorX);

  // EVAC6ParamID::kMaxLevelAutoReset
  int32 savedMaxLevelAutoReset = 0;
  if(!streamer.readInt32(savedMaxLevelAutoReset))
    savedMaxLevelAutoReset = DEFAULT_MAX_LEVEL_RESET_IN_SECONDS;
  setParamNormalized(EVAC6ParamID::kMaxLevelAutoReset,
                     normalizeDiscreteValue(MAX_LEVEL_AUTO_RESET_STEP_COUNT, savedMaxLevelAutoReset));

  // EVAC6ParamID::kLCDLeftChannel
  bool savedLeftChannelOn;
  if(!streamer.readBool(savedLeftChannelOn))
    savedLeftChannelOn = true;
  setParamNormalized(EVAC6ParamID::kLCDLeftChannel, normalizeBoolValue(savedLeftChannelOn));

  // EVAC6ParamID::kLCDRightChannel
  bool savedRightChannelOn;
  if(!streamer.readBool(savedRightChannelOn))
    savedRightChannelOn = true;
  setParamNormalized(EVAC6ParamID::kLCDRightChannel, normalizeBoolValue(savedRightChannelOn));

  // EVAC6ParamID::kLCDLiveView
  bool savedLiveView;
  if(!streamer.readBool(savedLiveView))
    savedLiveView = true;
  setParamNormalized(EVAC6ParamID::kLCDLiveView, normalizeBoolValue(savedLiveView));

  DLOG_F(INFO, "VAC6Controller::setComponentState => kSoftClippingLevel=%f, kLCDZoomFactorX=%f, kMaxLevelAutoReset=%d, kLCDLeftChannel=%d, kLCDRightChannel=%d, kLCDLiveView=%d",
         savedParamSoftLevelClipping,
         savedParamZoomFactorX,
         savedMaxLevelAutoReset,
         savedLeftChannelOn,
         savedRightChannelOn,
         savedLiveView);

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
      fMaxLevelState.onMessage(m);
      break;
    }

    case kLCDData_MID:
    {
      fLCDDisplayState.onMessage(m);
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