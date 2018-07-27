#include "RTState.h"

#include <sstream>

namespace pongasoft {
namespace VST {
namespace RT {

//------------------------------------------------------------------------
// RTState::RTState
//------------------------------------------------------------------------
RTState::RTState(Parameters const &iParameters) :
  fSaveStateOrder{iParameters.getRTSaveStateOrder()},
  fStateUpdate{NormalizedState{fSaveStateOrder.getParamCount()}, true},
  fLatestState{NormalizedState{fSaveStateOrder.getParamCount()}},
  fNormalizedStateRT{fSaveStateOrder.getParamCount()}
{
}

//------------------------------------------------------------------------
// RTState::RTState
//------------------------------------------------------------------------
void RTState::addRawParameter(std::shared_ptr<RTRawParameter> const &iParameter)
{
  ParamID paramID = iParameter->getParamID();

  DCHECK_F(iParameter != nullptr);
  DCHECK_F(fParameters.find(paramID) == fParameters.cend(), "duplicate paramID [%d]", paramID);
  DCHECK_F(!iParameter->getRawParamDef()->fUIOnly, "only RT parameter allowed");

  fParameters[paramID] = iParameter;
}

//------------------------------------------------------------------------
// IRTState::applyParameterChanges
//------------------------------------------------------------------------
bool RTState::applyParameterChanges(IParameterChanges &inputParameterChanges)
{
  int32 numParamsChanged = inputParameterChanges.getParameterCount();
  if(numParamsChanged <= 0)
    return false;

  bool stateChanged = false;

  for(int i = 0; i < numParamsChanged; ++i)
  {
    IParamValueQueue *paramQueue = inputParameterChanges.getParameterData(i);
    if(paramQueue != nullptr)
    {
      ParamValue value;
      int32 sampleOffset;
      int32 numPoints = paramQueue->getPointCount();

      // we read the "last" point (ignoring multiple changes for now)
      if(paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultOk)
      {
        auto item = fParameters.find(paramQueue->getParameterId());
        if(item != fParameters.cend())
        {
          stateChanged |= item->second->updateNormalizedValue(value);
        }
      }
    }
  }

  return stateChanged;
}

//------------------------------------------------------------------------
// RTState::computeLatestState
//------------------------------------------------------------------------
void RTState::computeLatestState()
{
  for(int i = 0; i < fNormalizedStateRT.fCount; i++)
  {
    auto paramID = fSaveStateOrder.fOrder[i];
    fNormalizedStateRT.set(i, fParameters.at(paramID)->getNormalizedValue());
  }

  fLatestState.set(fNormalizedStateRT);
}

//------------------------------------------------------------------------
// RTState::readNewState
//------------------------------------------------------------------------
tresult RTState::readNewState(IBStreamer &iStreamer)
{
  // YP Implementation note: It is OK to allocate memory here because this method is called by the GUI!!!
  NormalizedState normalizedState{fSaveStateOrder.getParamCount()};

  uint16 stateVersion;
  if(!iStreamer.readInt16u(stateVersion))
    stateVersion = fSaveStateOrder.fVersion;

  // TODO handle multiple versions
  if(stateVersion != fSaveStateOrder.fVersion)
  {
    DLOG_F(WARNING, "unexpected state version %d", stateVersion);
  }

  for(int i = 0; i < normalizedState.fCount; i++)
  {
    auto paramID = fSaveStateOrder.fOrder[i];
    // readNormalizedValue handles default values
    normalizedState.set(i, fParameters.at(paramID)->getRawParamDef()->readNormalizedValue(iStreamer));
  }

#ifdef VST_COMMON_DEBUG_LOGGING
  DLOG_F(INFO, "readNewState - %s", normalizedState.toString(fSaveStateOrder.fOrder.data()).c_str());
#endif

  fStateUpdate.push(normalizedState);

  return kResultOk;
}

//------------------------------------------------------------------------
// RTState::writeLatestState
//------------------------------------------------------------------------
tresult RTState::writeLatestState(IBStreamer &oStreamer)
{
  // YP Implementation note: It is OK to allocate memory here because this method is called by the GUI!!!
  NormalizedState normalizedState{fSaveStateOrder.getParamCount()};

  fLatestState.get(normalizedState);

  // write version for later upgrade
  oStreamer.writeInt16u(fSaveStateOrder.fVersion);

  for(int i = 0; i < normalizedState.fCount; i ++)
  {
    oStreamer.writeDouble(normalizedState.fValues[i]);
  }

#ifdef VST_COMMON_DEBUG_LOGGING
  DLOG_F(INFO, "writeLatestState - %s", normalizedState.toString(fSaveStateOrder.fOrder.data()).c_str());
#endif

  return kResultOk;
}

//------------------------------------------------------------------------
// RTState::beforeProcessing
//------------------------------------------------------------------------
bool RTState::beforeProcessing()
{
  if(fStateUpdate.pop(fNormalizedStateRT))
  {
    bool res = false;

    for(int i = 0; i < fNormalizedStateRT.fCount; i ++)
    {
      res |= fParameters.at(fSaveStateOrder.fOrder[i])->updateNormalizedValue(fNormalizedStateRT.fValues[i]);
    }

    return res;
  }
  return false;
}

//------------------------------------------------------------------------
// RTState::afterProcessing
//------------------------------------------------------------------------
void RTState::afterProcessing()
{
  bool stateChanged = false;
  for(auto &iter : fParameters)
  {
    stateChanged |= iter.second->resetPreviousValue();
  }

  // when the state has changed we update latest state for writeLatestState
  if(stateChanged)
  {
    computeLatestState();
  }
}

//------------------------------------------------------------------------
// RTState::init
//------------------------------------------------------------------------
tresult RTState::init()
{
#ifdef VST_COMMON_DEBUG_LOGGING
  DLOG_F(INFO, "RTState - Initialization");
#endif
  tresult result = kResultOk;
  for(int i = 0; i < fSaveStateOrder.getParamCount(); i++)
  {
    auto paramID = fSaveStateOrder.fOrder[i];
    // param exist
    if(fParameters.find(paramID) == fParameters.cend())
    {
      result = kResultFalse;
      DLOG_F(ERROR,
             "Expected parameter [%d] used in RTSaveStateOrder not registered",
             paramID);
    }
    if(fParameters.at(paramID)->getRawParamDef()->fTransient)
    {
      result = kResultFalse;
      DLOG_F(ERROR,
             "Parameter [%d] is marked transient => should not be part of save state order",
             paramID);
    }
  }
  DCHECK_F(result == kResultOk, "Issue with parameters... failing in development mode");

  if(result == kResultOk)
  {
    computeLatestState();
  }

  return result;
}

//------------------------------------------------------------------------
// RTState::NormalizedState::NormalizedState
//------------------------------------------------------------------------
RTState::NormalizedState::NormalizedState(int iCount)
{
  DCHECK_F(iCount >= 0);
  if(iCount > 0)
  {
    fCount = iCount;
    fValues = new ParamValue[fCount];

    for(int i = 0; i < fCount; i++)
    {
      fValues[i] = 0.0;
    }
  }
}

//------------------------------------------------------------------------
// RTState::NormalizedState::~NormalizedState
//------------------------------------------------------------------------
RTState::NormalizedState::~NormalizedState()
{
  delete[] fValues; // ok to delete nullptr
}

//------------------------------------------------------------------------
// RTState::NormalizedState::NormalizedState(&&) - Move constructor
//------------------------------------------------------------------------
RTState::NormalizedState::NormalizedState(RTState::NormalizedState &&other) noexcept
{
  fCount = other.fCount;
  fValues = other.fValues;

  other.fCount = 0;
  other.fValues = nullptr;
}

//------------------------------------------------------------------------
// RTState::NormalizedState::operator=
//------------------------------------------------------------------------
RTState::NormalizedState &RTState::NormalizedState::operator=(const RTState::NormalizedState &other)
{
  if(&other == this)
    return *this;

  // should not happen but sanity check!
  if(fCount != other.fCount)
    ABORT_F("no memory allocation allowed => aborting");
  else
  {
    std::copy(other.fValues, other.fValues + fCount, fValues);
  }

  return *this;
}

//------------------------------------------------------------------------
// RTState::NormalizedState::toString -- only for debug
//------------------------------------------------------------------------
std::string RTState::NormalizedState::toString(ParamID *iParamIDs) const
{
  std::ostringstream s;
  s << "NormalizedState{";
  for(int i = 0; i < fCount; i++)
  {
    if(i > 0)
      s << ", ";
    s << iParamIDs[i] << "=" << fValues[i];
  }
  s << "}";
  return s.str();
}

}
}
}