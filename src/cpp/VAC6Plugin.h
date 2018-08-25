#pragma once

#include "VAC6Model.h"
#include "VAC6CIDs.h"

#include <pongasoft/VST/Parameters.h>
#include <pongasoft/VST/RT/RTState.h>
#include <pongasoft/VST/GUI/GUIState.h>
#include <pongasoft/VST/Debug/ParamLine.h>

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

#ifndef NDEBUG
protected:
  // afterReadNewState
  void afterReadNewState(NormalizedState const *iState) override
  {
    DLOG_F(INFO, "RTState::read - %s", Debug::ParamLine::from(this, true).toString(*iState).c_str());
    //Debug::ParamTable::from(this, true).showCellSeparation().print(*iState, "RTState::read ---> ");
  }

  // beforeWriteNewState
  void beforeWriteNewState(NormalizedState const *iState) override
  {
    DLOG_F(INFO, "RTState::write - %s", Debug::ParamLine::from(this, true).toString(*iState).c_str());
    //Debug::ParamTable::from(this, true).showCellSeparation().print(*iState, "RTState::write ---> ");
  }
#endif

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

#ifndef NDEBUG
protected:
  // readGUIState
  tresult readGUIState(IBStreamer &iStreamer) override
  {
    tresult res = GUIState::readGUIState(iStreamer);
    if(res == kResultOk)
    {
      // swap the commented line to display either on a line or in a table
      DLOG_F(INFO, "GUIState::read - %s", Debug::ParamLine::from(this, true).toString().c_str());
      //Debug::ParamTable::from(this, true).showCellSeparation().print("GUIState::read ---> ");
    }
    return res;
  }

  // writeGUIState
  tresult writeGUIState(IBStreamer &oStreamer) const override
  {
    tresult res = GUIState::writeGUIState(oStreamer);
    if(res == kResultOk)
    {
      // swap the commented line to display either on a line or in a table
      DLOG_F(INFO, "GUIState::write - %s", Debug::ParamLine::from(this, true).toString().c_str());
      //Debug::ParamTable::from(this, true).showCellSeparation().print("GUIState::write ---> ");
    }
    return res;
  }
#endif

public:
  // messaging
  GUIJmbParam<HistoryData> fHistoryData;
};

}
}
}