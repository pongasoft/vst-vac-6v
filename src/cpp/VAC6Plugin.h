#pragma once

#include <pongasoft/VST/Plugin.h>
#include <pongasoft/VST/Parameter.h>
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

  void updateLCDInputX(ProcessData& iData, int iLCDInputX);
  void updateLCDHistoryOffset(ProcessData& iData, double iLCDHistoryOffset);
  void updateLCDLiveView(ProcessData& iData, bool iLCDLiveView);
};

class VAC6Plugin : public Plugin
{
public:
  VAC6Plugin(Type iType) :
    Plugin(iType),
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

  std::shared_ptr<Parameter<BooleanParamConverter>> fBypassParam;
  std::shared_ptr<Parameter<SoftClippingLevelParamConverter>> fSoftClippingLevelParam;
};


}
}
}