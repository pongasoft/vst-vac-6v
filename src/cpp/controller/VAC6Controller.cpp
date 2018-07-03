#include <vstgui4/vstgui/plugin-bindings/vst3editor.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <base/source/fstreamer.h>
#include <pluginterfaces/base/ustring.h>
#include "../logging/loguru.hpp"
#include "VAC6Controller.h"
#include "../VAC6CIDs.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////////////
// VAC6Controller::VAC6Controller
///////////////////////////////////////////
VAC6Controller::VAC6Controller() : EditController(),
                                   fXmlFile("VAC6.uidesc"),
                                   fHistoryState{std::make_shared<HistoryState>()},
                                   fMaxLevelState{fHistoryState},
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

  // the knob that changes the soft clipping level
  parameters.addParameter(STR16 ("Soft Clipping Level"), // title
                          nullptr, // units
                          0, // stepCount => continuous
                          SoftClippingLevel{DEFAULT_SOFT_CLIPPING_LEVEL}.getNormalizedParam(), // defaultNormalizedValue
                          ParameterInfo::ParameterFlags::kCanAutomate, // flags
                          EVAC6ParamID::kSoftClippingLevel, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("Sft Clp Lvl")); // shortTitle

  // the momentary button that resets the max level
  parameters.addParameter(STR16 ("Max Level Reset"), // title
                          nullptr, // units
                          1, // stepCount => 1 means toggle
                          BooleanParamConverter::normalize(false), // defaultNormalizedValue
                          ParameterInfo::ParameterFlags::kCanAutomate, // flags
                          EVAC6ParamID::kMaxLevelReset, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("Max Lvl Rst")); // shortTitle

  // the toggle for following
  parameters.addParameter(STR16 ("Max Level Follow"), // title
                          nullptr, // units
                          1, // stepCount => 1 means toggle
                          BooleanParamConverter::normalize(false), // defaultNormalizedValue
                          ParameterInfo::ParameterFlags::kCanAutomate, // flags
                          EVAC6ParamID::kMaxLevelFollow, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("Max Lvl Flw")); // shortTitle

  // An option menu to change the auto reset (from OFF to 10s)
  auto maxLevelModeParam = new StringListParameter(USTRING("Max Level Mode"), EVAC6ParamID::kMaxLevelMode);
  maxLevelModeParam->appendString(USTRING("Since Reset"));
  maxLevelModeParam->appendString(USTRING("In Window"));
  parameters.addParameter(maxLevelModeParam);

  // the zoom level knob
  parameters.addParameter(STR16 ("Zoom Level"), // title
                          nullptr, // units
                          0, // stepCount => continuous
                          DEFAULT_ZOOM_FACTOR_X, // defaultNormalizedValue
                          ParameterInfo::ParameterFlags::kCanAutomate, // flags
                          EVAC6ParamID::kLCDZoomFactorX, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("Zoom Lvl")); // shortTitle

  // on/off toggle to show/hide left channel
  parameters.addParameter(STR16 ("Left Channel"), // title
                          nullptr, // units
                          1, // stepCount => 1 means toggle
                          BooleanParamConverter::normalize(true), // defaultNormalizedValue
                          ParameterInfo::ParameterFlags::kCanAutomate, // flags
                          EVAC6ParamID::kLCDLeftChannel, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("L Chan")); // shortTitle

  // on/off toggle to show/hide right channel
  parameters.addParameter(STR16 ("Right Channel"), // title
                          nullptr, // units
                          1, // stepCount => 1 means toggle
                          BooleanParamConverter::normalize(true), // defaultNormalizedValue
                          ParameterInfo::ParameterFlags::kCanAutomate, // flags
                          EVAC6ParamID::kLCDRightChannel, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("R Chan")); // shortTitle

  // on/off toggle to show live view/pause
  parameters.addParameter(STR16 ("Live"), // title
                          nullptr, // units
                          1, // stepCount => 1 means toggle
                          BooleanParamConverter::normalize(true), // defaultNormalizedValue
                          0, // flags (state is not saved)
                          EVAC6ParamID::kLCDLiveView, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("Live")); // shortTitle

  // selected position on the screen when paused
  parameters.addParameter(STR16 ("Graph Select"), // title
                          nullptr, // units
                          MAX_ARRAY_SIZE, // stepCount => as many position as elements in the array
                          1.0, // defaultNormalizedValue => all the way to the right
                          0, // flags (state is not saved)
                          EVAC6ParamID::kLCDInputX, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("Gph Sel.")); // shortTitle

  // the scroll position (in percent)
  parameters.addParameter(STR16 ("Graph Scroll"), // title
                          nullptr, // units
                          0, // stepCount => continuous
                          1.0, // defaultNormalizedValue => all the way to the right
                          0, // flags (state is not saved)
                          EVAC6ParamID::kLCDHistoryOffset, // tag
                          kRootUnitId, // unitID => not using units at this stage
                          STR16 ("Gph Scr.")); // shortTitle

  fVSTParameters = std::make_shared<VSTParameters>(this);

  return result;
}

///////////////////////////////////////////
// VAC6Controller::terminate
///////////////////////////////////////////
tresult VAC6Controller::terminate()
{
  fVSTParameters = nullptr;

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
  auto customView = dynamic_cast<CustomView *>(view);
  if(customView != nullptr)
  {
    customView->initParameters(fVSTParameters);

    switch(customView->getCustomViewTag())
    {
      case EVAC6CustomViewTag::kMaxLevelValue:
        fMaxLevelState.assign(dynamic_cast<MaxLevelView *>(customView));
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
  setParamNormalized(EVAC6ParamID::kLCDZoomFactorX,
                     LCDZoomFactorXParamConverter::normalize(savedParamZoomFactorX));

  // EVAC6ParamID::kMaxLevelMode
  int32 savedMaxLevelMode = 0;
  if(!streamer.readInt32(savedMaxLevelMode))
    savedMaxLevelMode = DEFAULT_MAX_LEVEL_MODE;
  setParamNormalized(EVAC6ParamID::kMaxLevelMode,
                     MaxLevelModeParamConverter::normalize(static_cast<MaxLevelMode>(savedMaxLevelMode)));

  // EVAC6ParamID::kLCDLeftChannel
  bool savedLeftChannelOn;
  if(!streamer.readBool(savedLeftChannelOn))
    savedLeftChannelOn = true;
  setParamNormalized(EVAC6ParamID::kLCDLeftChannel, BooleanParamConverter::normalize(savedLeftChannelOn));

  // EVAC6ParamID::kLCDRightChannel
  bool savedRightChannelOn;
  if(!streamer.readBool(savedRightChannelOn))
    savedRightChannelOn = true;
  setParamNormalized(EVAC6ParamID::kLCDRightChannel, BooleanParamConverter::normalize(savedRightChannelOn));

  DLOG_F(INFO, "VAC6Controller::setComponentState => kSoftClippingLevel=%f, kLCDZoomFactorX=%f, kMaxLevelAutoReset=%d, kLCDLeftChannel=%d, kLCDRightChannel=%d",
         savedParamSoftLevelClipping,
         savedParamZoomFactorX,
         savedMaxLevelMode,
         savedLeftChannelOn,
         savedRightChannelOn);

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

  // EVAC6ParamID::kMaxLevelFollow
  {
    bool savedParam;
    if(!streamer.readBool(savedParam))
      savedParam = false;
    setParamNormalized(EVAC6ParamID::kMaxLevelFollow, BooleanParamConverter::normalize(savedParam));
  }

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

  streamer.writeBool(BooleanParamConverter::denormalize(getParamNormalized(EVAC6ParamID::kMaxLevelFollow)));

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
    case kLCDData_MID:
    {
      fHistoryState->onMessage(m);
      fMaxLevelState.onMessage(m);
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