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

/**
 * This class is the main entry point to parameters in the GUI (controller and views) world.
 */
class GUIParameters
{
  using PluginParameters = ::pongasoft::VST::Parameters;

public:
  // Constructor
  GUIParameters(HostParameters iHostParameters,
                PluginParameters const &iPluginParameters) :
    fHostParameters(iHostParameters),
    fPluginParameters{iPluginParameters}
  {}

  // getPluginParameters
  PluginParameters const &getPluginParameters() const { return fPluginParameters; }

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
   * This method is called from the GUI controller setComponentState method and reads the state coming from RT
   * and initializes the vst host parameters accordingly
   */
  tresult readRTState(IBStreamer &iStreamer);

  /**
   * This method is called from the GUI controller setState method and reads the state previously saved by the
   * GUI only (parameters that are ui only) and initializes the vst host parameters accordingly
   */
  tresult readGUIState(IBStreamer &iStreamer);

  /**
   * This method is called from the GUI controller getState method and writes the state specific to the
   * GUI only (parameters that are ui only), reading the values from the vst host parameters
   */
  tresult writeGUIState(IBStreamer &oStreamer) const;

  /**
   * @return the raw parameter given its id
   */
  std::unique_ptr<GUIRawParameter> getRawParameter(ParamID iParamID) const
  {
    return std::make_unique<GUIRawParameter>(iParamID, fHostParameters);
  }

  /**
   * The CustomView class automatically calls this method to get a handle of a ParamCxMgr used to register for interest
   * and obtain GUIParam instances. See CustomView::registerXXX methods.
   */
  std::unique_ptr<GUIParamCxMgr> createParamCxMgr() const;

protected:
  // readState according to iSaveStateOrder
  tresult readState(PluginParameters::SaveStateOrder const &iSaveStateOrder, IBStreamer &iStreamer);

private:
  HostParameters fHostParameters;
  PluginParameters const &fPluginParameters;
};

}
}
}
}

#endif // __PONGASOFT_VST_GUI_PARAMETERS_H__
