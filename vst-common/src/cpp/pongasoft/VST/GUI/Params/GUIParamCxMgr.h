#ifndef __PONGASOFT_VST_GUI_PARAM_CX_MGR_H__
#define __PONGASOFT_VST_GUI_PARAM_CX_MGR_H__

#include "GUIParameters.h"

namespace pongasoft {
namespace VST {
namespace GUI {
namespace Params {

class GUIParamCxMgr
{
public:
  /**
   * @return true if the param actually exists
   */
  bool exists(ParamID iParamID) const
  {
    return fParameters.exists(iParamID);
  }

  /**
   * Registers a raw parameter (no conversion)
   */
  std::unique_ptr<GUIRawParameter> registerGUIRawParam(ParamID iParamID,
                                                       GUIRawParameter::IChangeListener *iChangeListener = nullptr);

  /**
   * Generic register with any kind of conversion
   */
  template<typename T>
  std::unique_ptr<T> registerGUIParam(ParamID iParamID,
                                      GUIRawParameter::IChangeListener *iChangeListener = nullptr)
  {
    return std::make_unique<T>(registerGUIRawParam(iParamID, iChangeListener));
  }

  template<typename ParamConverter>
  GUIParamUPtr<ParamConverter> registerGUIParam(ParamDefSPtr<ParamConverter> iParamDef,
                                                GUIRawParameter::IChangeListener *iChangeListener = nullptr)
  {
    return std::make_unique<GUIParameter<ParamConverter>>(registerGUIRawParam(iParamDef->fParamID, iChangeListener));
  }

  friend class GUIParameters;

protected:
  explicit GUIParamCxMgr(GUIParameters const &iParameters) :
    fParameters{iParameters}
  {}

private:

  GUIParameters const &fParameters;

  // Maintains the connections for the listeners... will be automatically discarded in the destructor
  std::map<ParamID, std::unique_ptr<GUIRawParameter::Connection>> fParameterConnections;
};


}
}
}
}

#endif //__PONGASOFT_VST_GUI_PARAM_CX_MGR_H__
