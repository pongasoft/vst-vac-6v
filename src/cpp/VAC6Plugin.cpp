#include "VAC6Plugin.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

//------------------------------------------------------------------------
// VAC6Parameters::VAC6Parameters
//------------------------------------------------------------------------
VAC6Parameters::VAC6Parameters() : Parameters()
{
  // YP Note: registration order is important as it defines the order in which
  // they will be registered to the vst world hence for example the order in which they will be displayed
  // by Maschine

  // bypass
  fBypassParam =
    vst<BooleanParamConverter>(EVAC6ParamID::kBypass, STR16 ("Bypass"))
      .defaultValue(false)
      .flags(ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass)
      .shortTitle(STR16 ("Bypass"))
      .add();

  // the knob that changes the soft clipping level
  fSoftClippingLevelParam =
    vst<SoftClippingLevelParamConverter>(EVAC6ParamID::kSoftClippingLevel,
                                         STR16 ("Soft Clipping Level"))
      .defaultValue(SoftClippingLevel{DEFAULT_SOFT_CLIPPING_LEVEL})
      .shortTitle(STR16 ("Sft Clp Lvl"))
      .precision(2)
      .uiOnly()
      .add();

  // the zoom level knob
  fZoomFactorXParam =
    vst<LCDZoomFactorXParamConverter>(EVAC6ParamID::kLCDZoomFactorX, STR16 ("Zoom Level"))
      .defaultValue(DEFAULT_ZOOM_FACTOR_X)
      .shortTitle(STR16 ("Zoom Lvl"))
      .precision(1)
      .add();

  // on/off toggle to show live view/pause
  fLCDLiveViewParam =
    vst<BooleanParamConverter>(EVAC6ParamID::kLCDLiveView, STR16 ("Live"))
      .defaultValue(true)
      .shortTitle(STR16 ("Live"))
      .transient()
      .add();

  // the Gain1 knob
  fGain1Param =
    vst<GainParamConverter>(EVAC6ParamID::kGain1, STR16 ("Gain 1"))
      .defaultValue(DEFAULT_GAIN)
      .shortTitle(STR16 ("Gain1"))
      .precision(2)
      .add();

  // the Gain2 knob
  fGain2Param =
    vst<GainParamConverter>(EVAC6ParamID::kGain2, STR16 ("Gain 2"))
      .defaultValue(DEFAULT_GAIN)
      .shortTitle(STR16 ("Gain2"))
      .precision(2)
      .add();

  // on/off toggle to show/hide left channel
  fLeftChannelOnParam =
    vst<BooleanParamConverter>(EVAC6ParamID::kLCDLeftChannel, STR16 ("Left Channel"))
      .defaultValue(true)
      .shortTitle(STR16 ("L Chan"))
      .add();

  // on/off toggle to show/hide left channel
  fRightChannelOnParam =
    vst<BooleanParamConverter>(EVAC6ParamID::kLCDRightChannel, STR16 ("Right Channel"))
      .defaultValue(true)
      .shortTitle(STR16 ("R Chan"))
      .add();

  // the momentary button that resets the max level
  fMaxLevelResetParam =
    vst<BooleanParamConverter>(EVAC6ParamID::kMaxLevelReset, STR16 ("Max Level Reset"))
      .defaultValue(false)
      .shortTitle(STR16 ("Max Lvl Rst"))
      .transient()
      .add();

  // the toggle for the LCD marker representing since reset max level
  fSinceResetMarkerParam =
    vst<BooleanParamConverter>(EVAC6ParamID::kMaxLevelSinceResetMarker, STR16 ("Since Reset Marker"))
      .defaultValue(true)
      .shortTitle(STR16 ("Rst Mkr"))
      .uiOnly()
      .add();

  // the toggle for the LCD marker representing in window max level
  fInWindowMarkerParam =
    vst<BooleanParamConverter>(EVAC6ParamID::kMaxLevelInWindowMarker, STR16 ("In Window Marker"))
      .defaultValue(true)
      .shortTitle(STR16 ("Wdw Mkr"))
      .uiOnly()
      .add();

  // the toggle for gain filtering
  fGainFilterParam =
    vst<BooleanParamConverter>(EVAC6ParamID::kGainFilter, STR16 ("Gain Filter"))
      .defaultValue(DEFAULT_GAIN_FILTER)
      .shortTitle(STR16 ("Gn. Ft."))
      .add();

  // selected position on the screen when paused
  fLCDInputXParam =
    vst<LCDInputXParamConverter>(EVAC6ParamID::kLCDInputX, STR16 ("Graph Select"))
      .stepCount(MAX_ARRAY_SIZE + 1) // [-1, MAX_ARRAY_SIZE] -1 when nothing selected
      .defaultValue(LCD_INPUT_X_NOTHING_SELECTED) // not selected (-1)
      .flags(0) // state is not saved
      .transient()
      .add();

  // the scroll position (in percent)
  fLCDHistoryOffsetParam =
    vst<LCDHistoryOffsetParamConverter>(EVAC6ParamID::kLCDHistoryOffset, STR16 ("Graph Scroll"))
      .defaultValue(MAX_HISTORY_OFFSET) // all the way to the right
      .flags(0) // state is not saved
      .precision(0)
      .transient()
      .add();

  // history data
  fHistoryDataParam =
    ser<HistoryDataParamSerializer>(EVAC6ParamID::kHistoryData, STR16("HistoryData"))
      .transient()
      .uiOnly()
      .add();

  setRTSaveStateOrder(PROCESSOR_STATE_VERSION,
                      fZoomFactorXParam,
                      fLeftChannelOnParam,
                      fRightChannelOnParam,
                      fGain1Param,
                      fGain2Param,
                      fGainFilterParam,
                      fBypassParam);

  setGUISaveStateOrder(CONTROLLER_STATE_VERSION,
                       fSinceResetMarkerParam,
                       fInWindowMarkerParam,
                       fSoftClippingLevelParam);
}

}
}
}