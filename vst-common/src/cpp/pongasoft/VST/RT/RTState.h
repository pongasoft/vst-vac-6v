#ifndef __PONGASOFT_VST_RT_STATE_H__
#define __PONGASOFT_VST_RT_STATE_H__

#include "RTParameter.h"

#include <pongasoft/Utils/Concurrent/Concurrent.h>

#include <map>
#include <vector>

namespace pongasoft {
namespace VST {
namespace RT {

using namespace Utils;

//------------------------------------------------------------------------
// RTState - Templatized so as NOT to allocate memory at runtime in RT code
//------------------------------------------------------------------------
template<int ParamCount>
class RTState
{
protected:
  struct NormalizedState
  {
    ParamID fParamIDs[ParamCount]{};
    ParamValue fValues[ParamCount]{};
    int fCount{0};

    bool push(ParamID iParamID, ParamValue iParamValue)
    {
      DCHECK_F(fCount < ParamCount);

      if(fCount < ParamCount)
      {
        fParamIDs[fCount] = iParamID;
        fValues[fCount] = iParamValue;
        fCount++;
        return true;
      }
      return false;
    }
  };

public:
  
  template<typename ParamConverter>
  RTParamSPtr<ParamConverter> add(ParamDefSPtr<ParamConverter> iParamDef);

  void sanityCheck() const;

  void beforeProcessing();

  bool applyParameterChanges(IParameterChanges &inputParameterChanges);

  void afterProcessing();

  void readNewState(IBStreamer &iStreamer);

  void writeLatestState(IBStreamer &oStreamer);

protected:
  NormalizedState getNormalizedState() const;

  bool applyNormalizedState(NormalizedState const &iNormalizedState);

  // add raw parameter to the structures
  void addRawParameter(std::shared_ptr<RTRawParameter> iParameter);

private:
  // contains all the registered parameters (unique ID, will be checked on add)
  std::map<ParamID, std::shared_ptr<RTRawParameter>> fParameters{};
  // maintains the insertion order and keeps track of parameters saved by the RT
  ParamID fRTParamIDs[ParamCount]{};
  int fRTParamIDsCount = 0;

  Concurrent::WithSpinLock::SingleElementQueue<NormalizedState> fStateUpdate;
  Concurrent::WithSpinLock::AtomicValue<NormalizedState> fLatestState{NormalizedState{}};
};

//------------------------------------------------------------------------
// RTState::add
//------------------------------------------------------------------------
template<int ParamCount>
template<typename ParamConverter>
RTParamSPtr<ParamConverter> RTState<ParamCount>::add(ParamDefSPtr<ParamConverter> iParamDef)
{
  auto rtParam = std::make_shared<RTParameter<ParamConverter>>(iParamDef);
  addRawParameter(rtParam);
  return rtParam;
}

//------------------------------------------------------------------------
// RTState::addRawParameter
//------------------------------------------------------------------------
template<int ParamCount>
void RTState<ParamCount>::addRawParameter(std::shared_ptr<RTRawParameter> iParameter)
{
  ParamID paramID = iParameter->getParamID();

  DCHECK_F(iParameter != nullptr);
  DCHECK_F(fParameters.find(paramID) == fParameters.cend(), "duplicate paramID");
  DCHECK_F(!iParameter->getRawParamDef()->fUIOnly, "only RT parameter allowed");
  DCHECK_F(fRTParamIDsCount < ParamCount, "too many parameters... increase ParamCount");

  fParameters[paramID] = iParameter;
  fRTParamIDs[fRTParamIDsCount++] = paramID;
  fLatestState.set(getNormalizedState());
}

//------------------------------------------------------------------------
// RTState::getNormalizedState
//------------------------------------------------------------------------
template<int ParamCount>
typename RTState<ParamCount>::NormalizedState RTState<ParamCount>::getNormalizedState() const
{
  NormalizedState normalizedState{};

  for(int i = 0; i < fRTParamIDsCount; i++)
  {
    auto paramID = fRTParamIDs[i];
    normalizedState.push(paramID, fParameters.at(paramID)->getNormalizedValue());
  }

  return normalizedState;
}

//------------------------------------------------------------------------
// RTState::readNewState
//------------------------------------------------------------------------
template<int ParamCount>
void RTState<ParamCount>::readNewState(IBStreamer &iStreamer)
{
  NormalizedState normalizedState{};

  for(int i = 0; i < fRTParamIDsCount; i++)
  {
    auto paramID = fRTParamIDs[i];
    std::shared_ptr<RawParamDef> const &param = fParameters.at(paramID)->getRawParamDef();
    if(!param->fTransient)
      normalizedState.push(paramID, param->readNormalizedValue(iStreamer));
  }

  fStateUpdate.push(normalizedState);
}

//------------------------------------------------------------------------
// RTState::writeLatestState
//------------------------------------------------------------------------
template<int ParamCount>
void RTState<ParamCount>::writeLatestState(IBStreamer &oStreamer)
{
  auto state = fLatestState.get();

  for(int i = 0; i < state.fCount; i ++)
  {
    if(!fParameters.at(state.fParamIDs[i])->getRawParamDef()->fTransient)
      oStreamer.writeDouble(state.fValues[i]);
  }
}

//------------------------------------------------------------------------
// RTState::applyNormalizedState
//------------------------------------------------------------------------
template<int ParamCount>
bool RTState<ParamCount>::applyNormalizedState(RTState::NormalizedState const &iNormalizedState)
{
  bool res = false;

  for(int i = 0; i < iNormalizedState.fCount; i ++)
  {
    res |= fParameters.at(iNormalizedState.fParamIDs[i])->updateNormalizedValue(iNormalizedState.fValues[i]);
  }

  return res;
}

//------------------------------------------------------------------------
// RTState::applyParameterChanges
//------------------------------------------------------------------------
template<int ParamCount>
bool RTState<ParamCount>::applyParameterChanges(IParameterChanges &inputParameterChanges)
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
        stateChanged |= fParameters.at(paramQueue->getParameterId())->updateNormalizedValue(value);
      }
    }
  }

  return stateChanged;
}

//------------------------------------------------------------------------
// RTState::beforeProcessing
//------------------------------------------------------------------------
template<int ParamCount>
void RTState<ParamCount>::beforeProcessing()
{
  NormalizedState normalizedState{};
  if(fStateUpdate.pop(normalizedState))
  {
    applyNormalizedState(normalizedState);
  }
}

//------------------------------------------------------------------------
// RTState::afterProcessing
//------------------------------------------------------------------------
template<int ParamCount>
void RTState<ParamCount>::afterProcessing()
{
  bool stateChanged = false;
  for(int i = 0; i < fRTParamIDsCount; i++)
  {
    stateChanged |= fParameters.at(fRTParamIDs[i])->resetPreviousValue();
  }

  // when the state has changed we update it
  if(stateChanged)
    fLatestState.set(getNormalizedState());
}

//------------------------------------------------------------------------
// RTState::sanityCheck
//------------------------------------------------------------------------
template<int ParamCount>
void RTState<ParamCount>::sanityCheck() const
{
  if(fRTParamIDsCount != ParamCount)
  {
    DLOG_F(WARNING, "Mismatch param count: expected %d but was %d", ParamCount, fRTParamIDsCount);
  }
}


}
}
}

#endif // __PONGASOFT_VST_RT_STATE_H__