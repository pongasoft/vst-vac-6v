#pragma once

#include <pongasoft/VST/Plugin.h>
#include <pongasoft/VST/Parameters.h>
#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <pongasoft/VST/RT/RTState.h>
#include "VAC6Model.h"
#include "VAC6CIDs.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

struct State
{
  bool fBypass{false};
  double fZoomFactorX{DEFAULT_ZOOM_FACTOR_X};
  bool fLeftChannelOn{true};
  bool fRightChannelOn{true};
  bool fLCDLiveView{true};
  int fLCDInputX{MAX_LCD_INPUT_X};
  double fLCDHistoryOffset{MAX_HISTORY_OFFSET};
  Gain fGain1{};
  Gain fGain2{};
  bool fGainFilter{DEFAULT_GAIN_FILTER};

  void updateLCDInputX(Vst::ProcessData& iData, int iLCDInputX);
  void updateLCDHistoryOffset(Vst::ProcessData& iData, double iLCDHistoryOffset);
  void updateLCDLiveView(Vst::ProcessData& iData, bool iLCDLiveView);
};

class VAC6Parameters : public Parameters
{
public:
  VAC6Parameters() :
    Parameters(),
    fBypassParam{
      build<BooleanParamConverter>(EVAC6ParamID::kBypass, STR16 ("Bypass"))
        .defaultValue(false)
        .flags(ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass)
        .shortTitle(STR16 ("Bypass"))
        .add()
    },
    fSoftClippingLevelParam{
      build<SoftClippingLevelParamConverter>(EVAC6ParamID::kSoftClippingLevel,
                                             STR16 ("Soft Clipping Level"))
        .defaultValue(SoftClippingLevel{DEFAULT_SOFT_CLIPPING_LEVEL})
        .shortTitle(STR16 ("Sft Clp Lvl"))
        .precision(2)
        .uiOnly(true)
        .add()
    }
  {

  }

  ParamDefSPtr<BooleanParamConverter> fBypassParam;
  ParamDefSPtr<SoftClippingLevelParamConverter> fSoftClippingLevelParam;
};

using namespace RT;

class VAC6RTState : public RTState
{
public:
  explicit VAC6RTState(VAC6Parameters &iParams) :
    fBypass{add(iParams.fBypassParam)}
  {}

public:
  RTParamSPtr<BooleanParamConverter> fBypass;
};

}
}
}