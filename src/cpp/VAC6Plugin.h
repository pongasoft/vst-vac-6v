#pragma once

#include "VAC6Model.h"
#include "VAC6CIDs.h"

#include <pongasoft/VST/Parameters.h>
#include <pongasoft/VST/RT/RTState.h>

#include <pluginterfaces/vst/ivstaudioprocessor.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

class VAC6Parameters : public Parameters
{
public:
  VAC6Parameters() :
    Parameters(),
    // bypass
    fBypassParam{
      build<BooleanParamConverter>(EVAC6ParamID::kBypass, STR16 ("Bypass"))
        .defaultValue(false)
        .flags(ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass)
        .shortTitle(STR16 ("Bypass"))
        .add()
    },

    // the knob that changes the soft clipping level
    fSoftClippingLevelParam{
      build<SoftClippingLevelParamConverter>(EVAC6ParamID::kSoftClippingLevel,
                                             STR16 ("Soft Clipping Level"))
        .defaultValue(SoftClippingLevel{DEFAULT_SOFT_CLIPPING_LEVEL})
        .shortTitle(STR16 ("Sft Clp Lvl"))
        .precision(2)
        .uiOnly()
        .add()
    },

    // the zoom level knob
    fZoomFactorXParam{
      build<LCDZoomFactorXParamConverter>(EVAC6ParamID::kLCDZoomFactorX, STR16 ("Zoom Level"))
        .defaultValue(DEFAULT_ZOOM_FACTOR_X)
        .shortTitle(STR16 ("Zoom Lvl"))
        .precision(1)
        .add()
    },

    // on/off toggle to show live view/pause
    fLCDLiveViewParam{
      build<BooleanParamConverter>(EVAC6ParamID::kLCDLiveView, STR16 ("Live"))
        .defaultValue(true)
        .shortTitle(STR16 ("Live"))
        .transient()
        .add()
    },

    // the Gain1 knob
    fGain1Param{
      build<GainParamConverter>(EVAC6ParamID::kGain1, STR16 ("Gain 1"))
        .defaultValue(DEFAULT_GAIN)
        .shortTitle(STR16 ("Gain1"))
        .precision(2)
        .add()
    },

    // the Gain2 knob
    fGain2Param{
      build<GainParamConverter>(EVAC6ParamID::kGain2, STR16 ("Gain 1"))
        .defaultValue(DEFAULT_GAIN)
        .shortTitle(STR16 ("Gain2"))
        .precision(2)
        .add()
    },

    // on/off toggle to show/hide left channel
    fLeftChannelOnParam{
      build<BooleanParamConverter>(EVAC6ParamID::kLCDLeftChannel, STR16 ("Left Channel"))
        .defaultValue(true)
        .shortTitle(STR16 ("L Chan"))
        .add()
    },

    // on/off toggle to show/hide left channel
    fRightChannelOnParam{
      build<BooleanParamConverter>(EVAC6ParamID::kLCDRightChannel, STR16 ("Right Channel"))
        .defaultValue(true)
        .shortTitle(STR16 ("R Chan"))
        .add()
    },

    // the momentary button that resets the max level
    fMaxLevelResetParam{
      build<BooleanParamConverter>(EVAC6ParamID::kMaxLevelReset, STR16 ("Max Level Reset"))
        .defaultValue(false)
        .shortTitle(STR16 ("Max Lvl Rst"))
        .transient()
        .add()
    },

    // the toggle for the LCD marker representing since reset max level
    fSinceResetMarkerParam{
      build<BooleanParamConverter>(EVAC6ParamID::kMaxLevelSinceResetMarker, STR16 ("Since Reset Marker"))
        .defaultValue(true)
        .shortTitle(STR16 ("Rst Mkr"))
        .add()
    },

    // the toggle for the LCD marker representing in window max level
    fInWindowMarkerParam{
      build<BooleanParamConverter>(EVAC6ParamID::kMaxLevelInWindowMarker, STR16 ("In Window Marker"))
        .defaultValue(true)
        .shortTitle(STR16 ("Wdw Mkr"))
        .add()
    },

    // the toggle for gain filtering
    fGainFilterParam{
      build<BooleanParamConverter>(EVAC6ParamID::kGainFilter, STR16 ("Gain Filter"))
        .defaultValue(DEFAULT_GAIN_FILTER)
        .shortTitle(STR16 ("Gn. Ft."))
        .add()
    },

    // selected position on the screen when paused
    fLCDInputXParam{
      build<LCDInputXParamConverter>(EVAC6ParamID::kLCDInputX, STR16 ("Graph Select"))
        .stepCount(MAX_ARRAY_SIZE + 1) // [-1, MAX_ARRAY_SIZE] -1 when nothing selected
        .defaultValue(LCD_INPUT_X_NOTHING_SELECTED) // not selected (-1)
        .flags(0) // state is not saved
        .transient()
        .add()
    },

    // the scroll position (in percent)
    fLCDHistoryOffsetParam{
      build<LCDHistoryOffsetParamConverter>(EVAC6ParamID::kLCDHistoryOffset, STR16 ("Graph Scroll"))
        .defaultValue(MAX_HISTORY_OFFSET) // all the way to the right
        .flags(0) // state is not saved
        .precision(0)
        .transient()
        .add()
    }
  {
  }

  // saved
  ParamDefSPtr<BooleanParamConverter> fBypassParam;
  ParamDefSPtr<LCDZoomFactorXParamConverter> fZoomFactorXParam;
  ParamDefSPtr<GainParamConverter> fGain1Param;
  ParamDefSPtr<GainParamConverter> fGain2Param;
  ParamDefSPtr<BooleanParamConverter> fLeftChannelOnParam;
  ParamDefSPtr<BooleanParamConverter> fRightChannelOnParam;
  ParamDefSPtr<BooleanParamConverter> fGainFilterParam;

  // transient
  ParamDefSPtr<BooleanParamConverter> fLCDLiveViewParam;
  ParamDefSPtr<BooleanParamConverter> fMaxLevelResetParam;
  ParamDefSPtr<LCDInputXParamConverter> fLCDInputXParam;
  ParamDefSPtr<LCDHistoryOffsetParamConverter> fLCDHistoryOffsetParam;

  // UI Only
  ParamDefSPtr<SoftClippingLevelParamConverter> fSoftClippingLevelParam;
  ParamDefSPtr<BooleanParamConverter> fSinceResetMarkerParam;
  ParamDefSPtr<BooleanParamConverter> fInWindowMarkerParam;
};

using namespace RT;

class VAC6RTState : public RTState<11>
{
public:
  explicit VAC6RTState(VAC6Parameters &iParams) :
    fZoomFactorX{add(iParams.fZoomFactorXParam)},
    fLeftChannelOn{add(iParams.fLeftChannelOnParam)},
    fRightChannelOn{add(iParams.fRightChannelOnParam)},
    fGain1{add(iParams.fGain1Param)},
    fGain2{add(iParams.fGain2Param)},
    fGainFilter{add(iParams.fGainFilterParam)},
    fBypass{add(iParams.fBypassParam)},

    fLCDLiveView{add(iParams.fLCDLiveViewParam)},
    fMaxLevelReset{add(iParams.fMaxLevelResetParam)},
    fLCDInputX{add(iParams.fLCDInputXParam)},
    fLCDHistoryOffset{add(iParams.fLCDHistoryOffsetParam)}
  {
    sanityCheck();
  }

public:
  // saved state
  RTParam<LCDZoomFactorXParamConverter> fZoomFactorX;
  RTParam<BooleanParamConverter> fLeftChannelOn;
  RTParam<BooleanParamConverter> fRightChannelOn;
  RTParam<GainParamConverter> fGain1;
  RTParam<GainParamConverter> fGain2;
  RTParam<BooleanParamConverter> fGainFilter;
  RTParam<BooleanParamConverter> fBypass;

  // transient state
  RTParam<BooleanParamConverter> fLCDLiveView;
  RTParam<BooleanParamConverter> fMaxLevelReset;
  RTParam<LCDInputXParamConverter> fLCDInputX;
  RTParam<LCDHistoryOffsetParamConverter> fLCDHistoryOffset;
};

}
}
}