#pragma once

#include "VAC6Model.h"
#include "VAC6CIDs.h"

#include <pongasoft/VST/Parameters.h>
#include <pongasoft/VST/RT/RTState.h>
#include <pongasoft/VST/GUI/GUIState.h>

#include <pluginterfaces/vst/ivstaudioprocessor.h>

namespace pongasoft {
namespace VST {
namespace VAC6 {

class VAC6Parameters : public Parameters
{
public:
  VAC6Parameters();

  // saved
  VstParam<LCDZoomFactorXParamConverter> fZoomFactorXParam;
  VstParam<BooleanParamConverter> fLeftChannelOnParam;
  VstParam<BooleanParamConverter> fRightChannelOnParam;
  VstParam<GainParamConverter> fGain1Param;
  VstParam<GainParamConverter> fGain2Param;
  VstParam<BooleanParamConverter> fGainFilterParam;
  VstParam<BooleanParamConverter> fBypassParam;

  // transient
  VstParam<BooleanParamConverter> fLCDLiveViewParam;
  VstParam<BooleanParamConverter> fMaxLevelResetParam;
  VstParam<LCDInputXParamConverter> fLCDInputXParam;
  VstParam<LCDHistoryOffsetParamConverter> fLCDHistoryOffsetParam;

  // UI Only
  VstParam<SoftClippingLevelParamConverter> fSoftClippingLevelParam;
  VstParam<BooleanParamConverter> fSinceResetMarkerParam;
  VstParam<BooleanParamConverter> fInWindowMarkerParam;
};

using namespace RT;

class VAC6RTState : public RTState
{
public:
  explicit VAC6RTState(VAC6Parameters const &iParams) :
    RTState(iParams),
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
  }

public:
  // saved state
  RTVstParam<LCDZoomFactorXParamConverter> fZoomFactorX;
  RTVstParam<BooleanParamConverter> fLeftChannelOn;
  RTVstParam<BooleanParamConverter> fRightChannelOn;
  RTVstParam<GainParamConverter> fGain1;
  RTVstParam<GainParamConverter> fGain2;
  RTVstParam<BooleanParamConverter> fGainFilter;
  RTVstParam<BooleanParamConverter> fBypass;

  // transient state
  RTVstParam<BooleanParamConverter> fLCDLiveView;
  RTVstParam<BooleanParamConverter> fMaxLevelReset;
  RTVstParam<LCDInputXParamConverter> fLCDInputX;
  RTVstParam<LCDHistoryOffsetParamConverter> fLCDHistoryOffset;
};

using namespace GUI;

// no non vst parameters to handle => no need to inherit
using VAC6GUIState = GUIPluginState<VAC6Parameters>;

}
}
}