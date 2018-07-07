#include "VSTParameters.h"

namespace pongasoft {
namespace VST {
namespace GUI {


///////////////////////////////////////////
// RawParameter::Editor::Editor
///////////////////////////////////////////
RawParameter::Editor::Editor(ParamID iParamID, ParameterOwner *iParameterOwner) :
  fParamID{iParamID},
  fParameterOwner{iParameterOwner}
{
// DLOG_F(INFO, "RawParameter::Editor(%d)", fParamID);
  fParameterOwner->beginEdit(fParamID);
  fIsEditing = true;
  fInitialParamValue = fParameterOwner->getParamNormalized(fParamID);
}

///////////////////////////////////////////
// RawParameter::Editor::setValue
///////////////////////////////////////////
tresult RawParameter::Editor::setValue(ParamValue iValue)
{
  tresult res = kResultFalse;
  if(fIsEditing)
  {
    res = fParameterOwner->setParamNormalized(fParamID, iValue);
    if(res == kResultOk)
      fParameterOwner->performEdit(fParamID, fParameterOwner->getParamNormalized(fParamID));
  }
  return res;
}

///////////////////////////////////////////
// RawParameter::Editor::commit
///////////////////////////////////////////
tresult RawParameter::Editor::commit()
{
  if(fIsEditing)
  {
    fIsEditing = false;
    fParameterOwner->endEdit(fParamID);
    return kResultOk;
  }
  return kResultFalse;
}

///////////////////////////////////////////
// RawParameter::Editor::rollback
///////////////////////////////////////////
tresult RawParameter::Editor::rollback()
{
  if(fIsEditing)
  {
    setValue(fInitialParamValue);
    fIsEditing = false;
    fParameterOwner->endEdit(fParamID);
    return kResultOk;
  }
  return kResultFalse;
}

///////////////////////////////////////////
// RawParameter::Connection::Connection
///////////////////////////////////////////
RawParameter::Connection::Connection(ParamID iParamID,
                                     ParameterOwner *iParameterOwner,
                                     RawParameter::IChangeListener *iChangeListener)  :
  fParamID{iParamID},
  fParameterOwner{iParameterOwner},
  fChangeListener{iChangeListener}
{
  // DLOG_F(INFO, "RawParameter::Connection(%d)", fParamID);

  DCHECK_NOTNULL_F(fParameterOwner);

  fParameter = fParameterOwner->getParameterObject(fParamID);

  DCHECK_NOTNULL_F(fParameter);
  DCHECK_NOTNULL_F(fChangeListener);

  fParameter->addRef();
  fParameter->addDependent(this);
  fIsConnected = true;
}

///////////////////////////////////////////
// RawParameter::Connection::close
///////////////////////////////////////////
void RawParameter::Connection::close()
{
  if(fIsConnected)
  {
    fParameter->removeDependent(this);
    fParameter->release();
    fIsConnected = false;
  }
}

///////////////////////////////////////////
// RawParameter::Connection::update
///////////////////////////////////////////
void PLUGIN_API RawParameter::Connection::update(FUnknown *iChangedUnknown, Steinberg::int32 iMessage)
{
  if(iMessage == IDependent::kChanged)
  {
    fChangeListener->onParameterChange(fParamID, fParameterOwner->getParamNormalized(fParamID));
  }
}

///////////////////////////////////////////
// RawParameter::RawParameter
///////////////////////////////////////////
RawParameter::RawParameter(ParamID iParamID, ParameterOwner *iParameterOwner)  :
  fParamID{iParamID},
  fParameterOwner{iParameterOwner}
{
  // DLOG_F(INFO, "RawParameter::RawParameter(%d)", fParamID);
  DCHECK_NOTNULL_F(fParameterOwner);

  fParameter = fParameterOwner->getParameterObject(fParamID);
  DCHECK_NOTNULL_F(fParameter);
}

///////////////////////////////////////////
// VSTParametersManager::registerRawParameter
///////////////////////////////////////////
std::unique_ptr<RawParameter> VSTParametersManager::registerRawParameter(ParamID iParamID,
                                                                         RawParameter::IChangeListener *iChangeListener)
{
  auto parameter = fParameters->getRawParameter(iParamID);

  if(iChangeListener)
  {
    fParameterConnections[iParamID] = std::move(parameter->connect(iChangeListener));
  }

  return parameter;
}

///////////////////////////////////////////
// VSTParametersManager::registerBooleanParameter
///////////////////////////////////////////
std::unique_ptr<BooleanParameter>
VSTParametersManager::registerBooleanParameter(ParamID iParamID, RawParameter::IChangeListener *iChangeListener)
{
  return registerVSTParameter<BooleanParameter>(iParamID, iChangeListener);
}

///////////////////////////////////////////
// VSTParametersManager::registerPercentParameter
///////////////////////////////////////////
std::unique_ptr<PercentParameter>
VSTParametersManager::registerPercentParameter(ParamID iParamID, RawParameter::IChangeListener *iChangeListener)
{
  return registerRawParameter(iParamID, iChangeListener);
}

///////////////////////////////////////////
// VSTParametersManager::createManager
///////////////////////////////////////////
std::unique_ptr<VSTParametersManager> VSTParameters::createManager()
{
  // not using make_unique because protected constructor
  std::unique_ptr<VSTParametersManager> ptr{new VSTParametersManager(shared_from_this())};
  return ptr;
}

}
}
}