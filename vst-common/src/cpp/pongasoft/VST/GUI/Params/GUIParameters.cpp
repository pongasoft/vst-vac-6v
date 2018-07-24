#include "GUIParameters.h"
#include "GUIParamCxMgr.h"

namespace pongasoft {
namespace VST {
namespace GUI {
namespace Params {

//------------------------------------------------------------------------
// GUIParameters::registerVstParameters
//------------------------------------------------------------------------
void GUIParameters::registerVstParameters(Vst::ParameterContainer &iParameterContainer) const
{
  fPluginParameters.registerVstParameters(iParameterContainer);
}

//------------------------------------------------------------------------
// GUIParameters::createParamCxMgr
//------------------------------------------------------------------------
std::unique_ptr<GUIParamCxMgr> GUIParameters::createParamCxMgr() const
{
  return std::unique_ptr<GUIParamCxMgr>(new GUIParamCxMgr(*this));
}

//------------------------------------------------------------------------
// GUIParameters::readState
//------------------------------------------------------------------------
tresult GUIParameters::readState(Parameters::SaveStateOrder const &iSaveStateOrder, IBStreamer &iStreamer)
{
  uint16 stateVersion;
  if(!iStreamer.readInt16u(stateVersion))
    stateVersion = iSaveStateOrder.fVersion;

  // TODO handle multiple versions
  if(stateVersion != iSaveStateOrder.fVersion)
  {
    DLOG_F(WARNING, "unexpected state version %d", stateVersion);
  }

  for(auto paramID : iSaveStateOrder.fOrder)
  {
    auto param = fPluginParameters.getRawParamDef(paramID);
    ParamValue defaultNormalizedValue = param ? param->fDefaultNormalizedValue : 0.0;
    fHostParameters.setParamNormalized(paramID, iStreamer, defaultNormalizedValue);
  }

  return kResultOk;
}

//------------------------------------------------------------------------
// GUIParameters::readRTState
//------------------------------------------------------------------------
tresult GUIParameters::readRTState(IBStreamer &iStreamer)
{
  return readState(fPluginParameters.getRTSaveStateOrder(), iStreamer);
}


//------------------------------------------------------------------------
// GUIParameters::readGUIState
//------------------------------------------------------------------------
tresult GUIParameters::readGUIState(IBStreamer &iStreamer)
{
  return readState(fPluginParameters.getGUISaveStateOrder(), iStreamer);
}

//------------------------------------------------------------------------
// GUIParameters::writeGUIState
//------------------------------------------------------------------------
tresult GUIParameters::writeGUIState(IBStreamer &oStreamer) const
{
  auto sso = fPluginParameters.getGUISaveStateOrder();

  oStreamer.writeInt16u(sso.fVersion);

  for(auto paramID : sso.fOrder)
  {
    oStreamer.writeDouble(fHostParameters.getParamNormalized(paramID));
  }

  return kResultOk;
}

}
}
}
}