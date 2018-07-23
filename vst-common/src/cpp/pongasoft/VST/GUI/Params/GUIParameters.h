#ifndef __PONGASOFT_VST_GUI_PARAMETERS_H__
#define __PONGASOFT_VST_GUI_PARAMETERS_H__

#include <pongasoft/VST/Parameters.h>

#include <public.sdk/source/vst/vstparameters.h>
#include <public.sdk/source/vst/vsteditcontroller.h>

#include "GUIParameter.h"

namespace pongasoft {
namespace VST {
namespace GUI {
namespace Params {

using namespace Steinberg;

class GUIParamCxMgr;

class GUIParameters
{
  using PluginParameters = ::pongasoft::VST::Parameters;

public:
  GUIParameters(HostParameters iHostParameters,
                PluginParameters const &iPluginParameters) :
    fHostParameters(iHostParameters),
    fPluginParameters{iPluginParameters}
  {}

  /**
   * @return true if the param actually exists
   */
  inline bool exists(ParamID iParamID) const { return fHostParameters.exists(iParamID); }

  /**
   * This method is called from the GUI controller to register all the parameters to the ParameterContainer class
   * which is the class managing the parameters in the vst sdk
   */
  void registerVstParameters(Vst::ParameterContainer &iParameterContainer) const;

  /**
   * @return the raw parameter given its id
   */
  std::unique_ptr<GUIRawParameter> getRawParameter(ParamID iParamID) const
  {
    return std::make_unique<GUIRawParameter>(iParamID, fHostParameters);
  }

  std::unique_ptr<GUIParamCxMgr> createParamCxMgr() const;

private:
  HostParameters fHostParameters;
  PluginParameters const &fPluginParameters;
};

}
}
}
}

#endif // __PONGASOFT_VST_GUI_PARAMETERS_H__
