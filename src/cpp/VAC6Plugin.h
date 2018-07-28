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
  VAC6Parameters();

  // saved
  ParamDefSPtr<LCDZoomFactorXParamConverter> fZoomFactorXParam;
  ParamDefSPtr<BooleanParamConverter> fLeftChannelOnParam;
  ParamDefSPtr<BooleanParamConverter> fRightChannelOnParam;
  ParamDefSPtr<GainParamConverter> fGain1Param;
  ParamDefSPtr<GainParamConverter> fGain2Param;
  ParamDefSPtr<BooleanParamConverter> fGainFilterParam;
  ParamDefSPtr<BooleanParamConverter> fBypassParam;

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

class VAC6RTState : public RTState
{
public:
  explicit VAC6RTState(VAC6Parameters &iParams) :
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