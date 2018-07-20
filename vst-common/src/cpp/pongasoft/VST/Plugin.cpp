#include "Plugin.h"

namespace pongasoft {
namespace VST {

//------------------------------------------------------------------------
// Plugin::addRawParameter
//------------------------------------------------------------------------
void Plugin::addRawParameter(std::shared_ptr<RawParameter> iParameter)
{
  DCHECK_F(iParameter != nullptr);
  DCHECK_F(fParameters.find(iParameter->fParamID) == fParameters.cend());
  fParameters[iParameter->fParamID] = iParameter;
  if(iParameter->fUIOnly && fType == Plugin::Type::kGUI)
    fGUIParameters.emplace_back(iParameter);
  else
    fRTParameters.emplace_back(iParameter);
}

}
}