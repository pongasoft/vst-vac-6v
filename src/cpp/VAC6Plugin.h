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

  // used to communicate data from the processing to the UI
  JmbParam<HistoryData> fHistoryDataParam;
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
    fLCDHistoryOffset{add(iParams.fLCDHistoryOffsetParam)},

    fHistoryData{addJmbOut(iParams.fHistoryDataParam)}
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

  // messaging
  RTJmbOutParam<HistoryData> fHistoryData;
};

using namespace GUI;

class VAC6GUIState : public GUIPluginState<VAC6Parameters>
{
public:
  explicit VAC6GUIState(VAC6Parameters const &iParams) :
    GUIPluginState(iParams),
    fHistoryData{add(iParams.fHistoryDataParam)}
  {};

public:
  // messaging
  GUIJmbParam<HistoryData> fHistoryData;
};

}
}
}