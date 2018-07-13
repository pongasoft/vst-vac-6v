#include <vstgui4/vstgui/plugin-bindings/vst3editor.h>
#include "../logging/loguru.hpp"
#include "VAC6Controller.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

///////////////////////////////////
// Parameter
///////////////////////////////////
template<typename ParamConverter>
class Parameter : public Vst::Parameter
{
public:
  struct Builder
  {
    // builder methods
    Builder(ParamID iTag, const TChar* iTitle) : fTag{iTag}, fTitle{iTitle} {}
    Builder &units(const TChar *iUnits) { fUnits = iUnits; return *this; }
    Builder &defaultValue(typename ParamConverter::ParamType const &iDefaultValue) { fDefaultNormalizedValue = ParamConverter::normalize(iDefaultValue); return *this;}
    Builder &stepCount(int32 iStepCount) { fStepCount = iStepCount; return *this; }
    Builder &flags(int32 iFlags) { fFlags = iFlags; return *this; }
    Builder &unitID(int32 iUnitID) { fUnitID = iUnitID; return *this; }
    Builder &shortTitle(const TChar *iShortTitle) { fShortTitle = iShortTitle; return *this; }
    Builder &precision(int32 iPrecision) { fPrecision = iPrecision; return *this; }

    // method to call to add to parameter container
    Parameter<ParamConverter> *add(ParameterContainer &iParameterContainer)
    {
      auto parameter = create();
      iParameterContainer.addParameter(parameter);
      return parameter;
    }

    // parameter factory method
    virtual Parameter<ParamConverter> *create() const { return new Parameter(*this); }

    // fields
    ParamID fTag;
    const TChar *fTitle;
    const TChar *fUnits = nullptr;
    ParamValue fDefaultNormalizedValue = 0;
    int32 fStepCount = 0;
    int32 fFlags = ParameterInfo::kCanAutomate;
    UnitID fUnitID = kRootUnitId;
    const TChar *fShortTitle = nullptr;
    int32 fPrecision = 4;
  };

public:
  explicit Parameter(Builder const &iBuilder) :
    Vst::Parameter(iBuilder.fTitle,
                   iBuilder.fTag,
                   iBuilder.fUnits,
                   iBuilder.fDefaultNormalizedValue,
                   iBuilder.fStepCount,
                   iBuilder.fFlags,
                   iBuilder.fUnitID,
                   iBuilder.fShortTitle)
  {
    setPrecision(iBuilder.fPrecision);
  }

  /**
   * Using ParamConverter::toString
   */
  void toString(ParamValue iNormalizedValue, String128 iString) const override
  {
    ParamConverter::toString(ParamConverter::denormalize(iNormalizedValue), iString, getPrecision());
  }
};

using BooleanParameter = Parameter<BooleanParamConverter>;

///////////////////////////////////////////
// VAC6Controller::VAC6Controller
///////////////////////////////////////////
VAC6Controller::VAC6Controller() : EditController(),
                                   fXmlFile("VAC6.uidesc"),
                                   fVSTParameters{nullptr},
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

  // bypass
  BooleanParameter::Builder(EVAC6ParamID::kBypass, STR16 ("Bypass"))
    .defaultValue(false)
    .flags(ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass)
    .shortTitle(STR16 ("Bypass"))
    .add(parameters);

  // the knob that changes the soft clipping level
  Parameter<SoftClippingLevelParamConverter>::Builder(EVAC6ParamID::kSoftClippingLevel, STR16 ("Soft Clipping Level"))
    .defaultValue(SoftClippingLevel{DEFAULT_SOFT_CLIPPING_LEVEL})
    .shortTitle(STR16 ("Sft Clp Lvl"))
    .precision(2)
    .add(parameters);

  // the zoom level knob
  Parameter<LCDZoomFactorXParamConverter>::Builder(EVAC6ParamID::kLCDZoomFactorX, STR16 ("Zoom Level"))
    .defaultValue(DEFAULT_ZOOM_FACTOR_X)
    .shortTitle(STR16 ("Zoom Lvl"))
    .precision(1)
    .add(parameters);

  // on/off toggle to show live view/pause
  BooleanParameter::Builder(EVAC6ParamID::kLCDLiveView, STR16 ("Live"))
    .defaultValue(true)
    .shortTitle(STR16 ("Live"))
    .add(parameters);

  // the Gain1 knob
  Parameter<GainParamConverter>::Builder(EVAC6ParamID::kGain1, STR16 ("Gain 1"))
    .defaultValue(DEFAULT_GAIN)
    .shortTitle(STR16 ("Gain1"))
    .precision(2)
    .add(parameters);

  // the Gain2 knob
  Parameter<GainParamConverter>::Builder(EVAC6ParamID::kGain2, STR16 ("Gain 2"))
    .defaultValue(DEFAULT_GAIN)
    .shortTitle(STR16 ("Gain2"))
    .precision(2)
    .add(parameters);

  // on/off toggle to show/hide left channel
  BooleanParameter::Builder(EVAC6ParamID::kLCDLeftChannel, STR16 ("Left Channel"))
    .defaultValue(true)
    .shortTitle(STR16 ("L Chan"))
    .add(parameters);

  // on/off toggle to show/hide right channel
  BooleanParameter::Builder(EVAC6ParamID::kLCDRightChannel, STR16 ("Right Channel"))
    .defaultValue(true)
    .shortTitle(STR16 ("R Chan"))
    .add(parameters);

  // the momentary button that resets the max level
  BooleanParameter::Builder(EVAC6ParamID::kMaxLevelReset, STR16 ("Max Level Reset"))
    .defaultValue(false)
    .shortTitle(STR16 ("Max Lvl Rst"))
    .add(parameters);

  // the toggle for the LCD marker representing since reset max level
  BooleanParameter::Builder(EVAC6ParamID::kMaxLevelSinceResetMarker, STR16 ("Since Reset Marker"))
    .defaultValue(true)
    .shortTitle(STR16 ("Rst Mkr"))
    .add(parameters);

  // the toggle for the LCD marker representing in window max level
  BooleanParameter::Builder(EVAC6ParamID::kMaxLevelInWindowMarker, STR16 ("In Window Marker"))
    .defaultValue(true)
    .shortTitle(STR16 ("Wdw Mkr"))
    .add(parameters);

  // the toggle for gain filtering
  BooleanParameter::Builder(EVAC6ParamID::kGainFilter, STR16 ("Gain Filter"))
    .defaultValue(DEFAULT_GAIN_FILTER)
    .shortTitle(STR16 ("Gn. Ft."))
    .add(parameters);

  // selected position on the screen when paused
  Parameter<LCDInputXParamConverter>::Builder(EVAC6ParamID::kLCDInputX, STR16 ("Graph Select"))
    .stepCount(MAX_ARRAY_SIZE + 1) // [-1, MAX_ARRAY_SIZE] -1 when nothing selected
    .defaultValue(LCD_INPUT_X_NOTHING_SELECTED) // not selected (-1)
    .flags(0) // state is not saved
    .add(parameters);

  // the scroll position (in percent)
  Parameter<LCDHistoryOffsetParamConverter>::Builder(EVAC6ParamID::kLCDHistoryOffset, STR16 ("Graph Scroll"))
    .defaultValue(MAX_HISTORY_OFFSET) // all the way to the right
    .flags(0) // state is not saved
    .precision(0)
    .add(parameters);

  fVSTParameters = std::make_shared<VSTParameters>(this);
  fViewFactory = new CustomUIViewFactory(fVSTParameters);

  return result;
}

///////////////////////////////////////////
// VAC6Controller::terminate
///////////////////////////////////////////
tresult VAC6Controller::terminate()
{
  delete fViewFactory;
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

  uint16 stateVersion;
  if(!streamer.readInt16u(stateVersion))
    stateVersion = PROCESSOR_STATE_VERSION;

  if(stateVersion != PROCESSOR_STATE_VERSION)
  {
    DLOG_F(WARNING, "unexpected processor state version %d", stateVersion);
  }

  setParamNormalized<LCDZoomFactorXParamConverter>(EVAC6ParamID::kLCDZoomFactorX, streamer, DEFAULT_ZOOM_FACTOR_X);
  setParamNormalized<BooleanParamConverter>(EVAC6ParamID::kLCDLeftChannel, streamer, true);
  setParamNormalized<BooleanParamConverter>(EVAC6ParamID::kLCDRightChannel, streamer, true);
  setParamNormalized<GainParamConverter>(EVAC6ParamID::kGain1, streamer, DEFAULT_GAIN);
  setParamNormalized<GainParamConverter>(EVAC6ParamID::kGain2, streamer, DEFAULT_GAIN);
  setParamNormalized<BooleanParamConverter>(EVAC6ParamID::kGainFilter, streamer, DEFAULT_GAIN_FILTER);
  setParamNormalized<BooleanParamConverter>(EVAC6ParamID::kBypass, streamer, false);

  return kResultOk;
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

  uint16 stateVersion;
  if(!streamer.readInt16u(stateVersion))
    stateVersion = CONTROLLER_STATE_VERSION;

  if(stateVersion != CONTROLLER_STATE_VERSION)
  {
    DLOG_F(WARNING, "unexpected controller state version %d", stateVersion);
  }

  setParamNormalized<BooleanParamConverter>(EVAC6ParamID::kMaxLevelSinceResetMarker, streamer, true);
  setParamNormalized<BooleanParamConverter>(EVAC6ParamID::kMaxLevelInWindowMarker, streamer, true);
  setParamNormalized<SoftClippingLevelParamConverter>(EVAC6ParamID::kSoftClippingLevel, streamer, SoftClippingLevel{DEFAULT_SOFT_CLIPPING_LEVEL});

  return kResultOk;
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

  // write version for later upgrade
  streamer.writeInt16u(CONTROLLER_STATE_VERSION);

  streamer.writeDouble(getParamNormalized(EVAC6ParamID::kMaxLevelSinceResetMarker));
  streamer.writeDouble(getParamNormalized(EVAC6ParamID::kMaxLevelInWindowMarker));
  streamer.writeDouble(getParamNormalized(EVAC6ParamID::kSoftClippingLevel));

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