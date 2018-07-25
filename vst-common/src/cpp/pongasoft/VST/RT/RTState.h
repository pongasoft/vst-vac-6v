#ifndef __PONGASOFT_VST_RT_STATE_H__
#define __PONGASOFT_VST_RT_STATE_H__

#include "RTParameter.h"

#include <pongasoft/Utils/Concurrent/Concurrent.h>
#include <pongasoft/VST/Parameters.h>

#include <map>
#include <vector>
#include <array>
#include <string>
#include <sstream>

namespace pongasoft {
namespace VST {
namespace RT {

using namespace Utils;

class IRTState
{
public:
  /**
   * This method is called for each parameter managed by RTState. The order in which this method is called is
   * important and reflects the order that will be used when reading/writing state to the stream
   */
  template<typename ParamConverter>
  RTParam<ParamConverter> add(ParamDefSPtr<ParamConverter> iParamDef);

  /**
   * This method should be call at the beginning of the process(ProcessData &data) method before doing anything else.
   * The goal of this method is to update the current state with a state set by the UI (typical use case is to
   * initialize the plugin when being loaded) */
  virtual void beforeProcessing() = 0;

  /**
   * This method should be called in every frame when there are parameter changes to update this state accordingly
   */
  bool applyParameterChanges(IParameterChanges &inputParameterChanges);

  /**
   * This method should be called at the end of process(ProcessData &data) method. It will update the previous state
   * to the current one and save the latest changes (if necessary) so that it is accessible via writeLatestState.
   */
  virtual void afterProcessing() = 0;

  /**
   * This method should be called from Processor::setState to update this state to the state stored in the stream.
   * Note that this method is called from the UI thread so the update is queued until the next frame.
   */
  virtual tresult readNewState(IBStreamer &iStreamer) = 0;

  /**
   * This method should be called from Processor::getState to store the latest state to the stream. Note that this
   * method is called from the UI thread and gets the "latest" state as of the end of the last frame.
   */
  virtual tresult writeLatestState(IBStreamer &oStreamer) = 0;

protected:
  // contains all the registered parameters (unique ID, will be checked on add)
  std::map<ParamID, std::shared_ptr<RTRawParameter>> fParameters{};

  // add raw parameter to the structures
  void addRawParameter(std::shared_ptr<RTRawParameter> const &iParameter);
};

/**
 * Encapsulates the state managed by the Real Time (RT).
 *
 * @tparam SavedParamCount Templatized so as NOT to allocate memory at runtime in RT code
 */
template<int SavedParamCount>
class RTState : public IRTState
{
protected:
  // Internal structure that gets exchanged between the UI thread and the RT thread (no memory allocation)
  struct NormalizedState
  {
    uint16 fVersion{0};
    ParamID fParamIDs[SavedParamCount]{};
    ParamValue fValues[SavedParamCount]{};
    int fCount{0};

    bool push(ParamID iParamID, ParamValue iParamValue)
    {
      DCHECK_F(fCount < SavedParamCount);

      if(fCount < SavedParamCount)
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

  // Internal structure which maintains the order for storing/reading the state
  struct SaveStateOrder
  {
    uint16 fVersion{0};
    ParamID fOrder[SavedParamCount]{};
    int fCount{0};
  };

public:

  RTState(Parameters const &iParameters);

  /**
   * Call this method after adding all the parameters */
  void init();

  /**
   * This method should be call at the beginning of the process(ProcessData &data) method before doing anything else.
   * The goal of this method is to update the current state with a state set by the UI (typical use case is to
   * initialize the plugin when being loaded) */
  void beforeProcessing() override;

  /**
   * This method should be called at the end of process(ProcessData &data) method. It will update the previous state
   * to the current one and save the latest changes (if necessary) so that it is accessible via writeLatestState.
   */
  void afterProcessing() override;

  /**
   * This method should be called from Processor::setState to update this state to the state stored in the stream.
   * Note that this method is called from the UI thread so the update is queued until the next frame.
   */
  tresult readNewState(IBStreamer &iStreamer) override;

  /**
   * This method should be called from Processor::getState to store the latest state to the stream. Note that this
   * method is called from the UI thread and gets the "latest" state as of the end of the last frame.
   */
  tresult writeLatestState(IBStreamer &oStreamer) override;

protected:
  NormalizedState getNormalizedState() const;

  bool applyNormalizedState(NormalizedState const &iNormalizedState);

private:
  // maintains the order for storing/reading the state
  SaveStateOrder fSaveStateOrder{};

  // this queue is used to propagate a Processor::setState call (made from the UI thread) to this state
  // the check happens in beforeProcessing
  Concurrent::WithSpinLock::SingleElementQueue<NormalizedState> fStateUpdate;

  // this atomic value always hold the most current (and consistent) version of this state so that the UI thread
  // can access it in Processor::getState. It is updated in afterProcessing.
  Concurrent::WithSpinLock::AtomicValue<NormalizedState> fLatestState{NormalizedState{}};
};

//------------------------------------------------------------------------
// RTState::RTState
//------------------------------------------------------------------------
template<int SavedParamCount>
RTState<SavedParamCount>::RTState(Parameters const &iParameters)
{
  auto sso = iParameters.getRTSaveStateOrder();
  DCHECK_F(sso.fOrder.size() <= SavedParamCount,
           "RTState<%d> too small. Should be RTState<%lu>", SavedParamCount, sso.fOrder.size());
  std::copy(sso.fOrder.cbegin(), sso.fOrder.cend(), fSaveStateOrder.fOrder);
  fSaveStateOrder.fCount = static_cast<int>(sso.fOrder.size());
  fSaveStateOrder.fVersion = sso.fVersion;
}

//------------------------------------------------------------------------
// IRTState::add
//------------------------------------------------------------------------
template<typename ParamConverter>
RTParam<ParamConverter> IRTState::add(ParamDefSPtr<ParamConverter> iParamDef)
{
  auto rtParam = std::make_shared<RTParameter<ParamConverter>>(iParamDef);
  addRawParameter(rtParam);
  return rtParam;
}

//------------------------------------------------------------------------
// RTState::getNormalizedState
//------------------------------------------------------------------------
template<int SavedParamCount>
typename RTState<SavedParamCount>::NormalizedState RTState<SavedParamCount>::getNormalizedState() const
{
  NormalizedState normalizedState{};

  normalizedState.fVersion = fSaveStateOrder.fVersion;

  for(int i = 0; i < fSaveStateOrder.fCount; i++)
  {
    auto paramID = fSaveStateOrder.fOrder[i];
    normalizedState.push(paramID, fParameters.at(paramID)->getNormalizedValue());
  }

  return normalizedState;
}

//------------------------------------------------------------------------
// RTState::readNewState
//------------------------------------------------------------------------
template<int SavedParamCount>
tresult RTState<SavedParamCount>::readNewState(IBStreamer &iStreamer)
{
  NormalizedState normalizedState{};

  uint16 stateVersion;
  if(!iStreamer.readInt16u(stateVersion))
    stateVersion = fSaveStateOrder.fVersion;

  // TODO handle multiple versions
  if(stateVersion != fSaveStateOrder.fVersion)
  {
    DLOG_F(WARNING, "unexpected state version %d", stateVersion);
  }

  normalizedState.fVersion = stateVersion;

  for(int i = 0; i < fSaveStateOrder.fCount; i++)
  {
    auto paramID = fSaveStateOrder.fOrder[i];
    normalizedState.push(paramID, fParameters.at(paramID)->getRawParamDef()->readNormalizedValue(iStreamer));
  }

#ifdef VST_COMMON_DEBUG_LOGGING
  DLOG_F(INFO, "readNewState - %s", normalizedState.toString().c_str());
#endif

  fStateUpdate.push(normalizedState);

  return kResultOk;
}

//------------------------------------------------------------------------
// RTState::writeLatestState
//------------------------------------------------------------------------
template<int SavedParamCount>
tresult RTState<SavedParamCount>::writeLatestState(IBStreamer &oStreamer)
{
  auto state = fLatestState.get();

  // write version for later upgrade
  oStreamer.writeInt16u(state.fVersion);

  for(int i = 0; i < state.fCount; i ++)
  {
    if(!fParameters.at(state.fParamIDs[i])->getRawParamDef()->fTransient)
      oStreamer.writeDouble(state.fValues[i]);
  }

#ifdef VST_COMMON_DEBUG_LOGGING
  DLOG_F(INFO, "writeLatestState - %s", state.toString().c_str());
#endif

  return kResultOk;
}

//------------------------------------------------------------------------
// RTState::applyNormalizedState
//------------------------------------------------------------------------
template<int SavedParamCount>
bool RTState<SavedParamCount>::applyNormalizedState(RTState::NormalizedState const &iNormalizedState)
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
// RTState::beforeProcessing
//------------------------------------------------------------------------
template<int SavedParamCount>
void RTState<SavedParamCount>::beforeProcessing()
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
template<int SavedParamCount>
void RTState<SavedParamCount>::afterProcessing()
{
  bool stateChanged = false;
  for(auto &iter : fParameters)
  {
    stateChanged |= iter.second->resetPreviousValue();
  }

  // when the state has changed we update it
  if(stateChanged)
  {
//    DLOG_F(INFO, "afterProcessing - %s", getNormalizedState().toString().c_str());
    fLatestState.set(getNormalizedState());
  }
}

//------------------------------------------------------------------------
// RTState::init
//------------------------------------------------------------------------
template<int SavedParamCount>
void RTState<SavedParamCount>::init()
{
  if(fSaveStateOrder.fCount != SavedParamCount)
  {
    DLOG_F(WARNING, "RTState<%d> mismatch => Should be RTState<%d>", SavedParamCount, fSaveStateOrder.fCount);
  }
  else
  {
    DLOG_F(INFO, "RTState - parameters registered count... checked");
  }

  for(int i = 0; i < fSaveStateOrder.fCount; i++)
  {
    auto paramID = fSaveStateOrder.fOrder[i];
    DCHECK_F(fParameters.find(paramID) != fParameters.cend(),
             "Expected parameter [%d] used in RTSaveStateOrder not registered",
             paramID);
  }
  DLOG_F(INFO, "RTState - all parameters used in RTSaveStateOrder registered... checked");

  fLatestState.set(getNormalizedState());
}



}
}
}

#endif // __PONGASOFT_VST_RT_STATE_H__