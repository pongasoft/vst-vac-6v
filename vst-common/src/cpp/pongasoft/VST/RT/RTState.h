#ifndef __PONGASOFT_VST_RT_STATE_H__
#define __PONGASOFT_VST_RT_STATE_H__

#include "RTParameter.h"

#include <pongasoft/Utils/Concurrent/Concurrent.h>

#include <map>
#include <vector>
#include <string>
#include <sstream>

namespace pongasoft {
namespace VST {
namespace RT {

using namespace Utils;

/**
 * Encapsulates the state managed by the Real Time (RT).
 *
 * @tparam ParamCount Templatized so as NOT to allocate memory at runtime in RT code
 */
template<int ParamCount>
class RTState
{
protected:
  // Internal structure that gets exchanged between the UI thread and the RT thread (no memory allocation)
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

#ifndef NDEBUG
    std::string toString() const
    {
      std::ostringstream s;
      s << "NormalizedState{";
      for(int i = 0; i < fCount; i++)
      {
        s << fParamIDs[i] << "=" << fValues[i] << ",";
      }
      s << "}";
      return s.str();
    }
  };
#endif

public:

  /**
   * This method is called for each parameter managed by RTState. The order in which this method is called is
   * important and reflects the order that will be used when reading/writing state to the stream
   */
  template<typename ParamConverter>
  RTParam<ParamConverter> add(ParamDefSPtr<ParamConverter> iParamDef);

  /**
   * Call this method is the constructor to check that ParamCount matches the number of params registered */
  void sanityCheck() const;

  /**
   * This method should be call at the beginning of the process(ProcessData &data) method before doing anything else.
   * The goal of this method is to update the current state with a state set by the UI (typical use case is to
   * initialize the plugin when being loaded) */
  void beforeProcessing();

  /**
   * This method should be called in every frame when there are parameter changes to update this state accordingly
   */
  bool applyParameterChanges(IParameterChanges &inputParameterChanges);

  /**
   * This method should be called at the end of process(ProcessData &data) method. It will update the previous state
   * to the current one and save the latest changes (if necessary) so that it is accessible via writeLatestState.
   */
  void afterProcessing();

  /**
   * This method should be called from Processor::setState to update this state to the state stored in the stream.
   * Note that this method is called from the UI thread so the update is queued until the next frame.
   */
  void readNewState(IBStreamer &iStreamer);

  /**
   * This method should be called from Processor::getState to store the latest state to the stream. Note that this
   * method is called from the UI thread and gets the "latest" state as of the end of the last frame.
   */
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

  // this queue is used to propagate a Processor::setState call (made from the UI thread) to this state
  // the check happens in beforeProcessing
  Concurrent::WithSpinLock::SingleElementQueue<NormalizedState> fStateUpdate;

  // this atomic value always hold the most current (and consistent) version of this state so that the UI thread
  // can access it in Processor::getState. It is updated in afterProcessing.
  Concurrent::WithSpinLock::AtomicValue<NormalizedState> fLatestState{NormalizedState{}};
};

//------------------------------------------------------------------------
// RTState::add
//------------------------------------------------------------------------
template<int ParamCount>
template<typename ParamConverter>
RTParam<ParamConverter> RTState<ParamCount>::add(ParamDefSPtr<ParamConverter> iParamDef)
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

  // we need to update the latest state since a parameter was added
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

//  DLOG_F(INFO, "readNewState - %s", normalizedState.toString().c_str());

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

//  DLOG_F(INFO, "writeLatestState - %s", state.toString().c_str());
}

//------------------------------------------------------------------------
// RTState::applyNormalizedState
//------------------------------------------------------------------------
template<int ParamCount>
bool RTState<ParamCount>::applyNormalizedState(RTState::NormalizedState const &iNormalizedState)
{
//  DLOG_F(INFO, "applyNormalizedState - %s", iNormalizedState.toString().c_str());

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
  {
//    DLOG_F(INFO, "afterProcessing - %s", getNormalizedState().toString().c_str());
    fLatestState.set(getNormalizedState());
  }
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