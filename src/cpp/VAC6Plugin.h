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
  VstParam<Percent> fZoomFactorXParam;
  VstParam<bool> fLeftChannelOnParam;
  VstParam<bool> fRightChannelOnParam;
  VstParam<Gain> fGain1Param;
  VstParam<Gain> fGain2Param;
  VstParam<bool> fGainFilterParam;
  VstParam<bool> fBypassParam;

  // transient
  VstParam<bool> fLCDLiveViewParam;
  VstParam<bool> fMaxLevelResetParam;
  VstParam<int> fLCDInputXParam;
  VstParam<Percent> fLCDHistoryOffsetParam;

  // UI Only
  VstParam<SoftClippingLevel> fSoftClippingLevelParam;
  VstParam<bool> fSinceResetMarkerParam;
  VstParam<bool> fInWindowMarkerParam;
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
  RTVstParam<Percent> fZoomFactorX;
  RTVstParam<bool> fLeftChannelOn;
  RTVstParam<bool> fRightChannelOn;
  RTVstParam<Gain> fGain1;
  RTVstParam<Gain> fGain2;
  RTVstParam<bool> fGainFilter;
  RTVstParam<bool> fBypass;

  // transient state
  RTVstParam<bool> fLCDLiveView;
  RTVstParam<bool> fMaxLevelReset;
  RTVstParam<int> fLCDInputX;
  RTVstParam<Percent> fLCDHistoryOffset;
};

using namespace GUI;

// no non vst parameters to handle => no need to inherit
using VAC6GUIState = GUIPluginState<VAC6Parameters>;

}
}
}