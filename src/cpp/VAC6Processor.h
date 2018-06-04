#pragma once

#include <public.sdk/source/vst/vstaudioeffect.h>
#include "VAC6Constants.h"
#include <base/source/timer.h>

namespace pongasoft {
namespace VST {

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace VAC6;

class VAC6Processor : public AudioEffect, ITimerCallback
{
public:
  VAC6Processor();

  ~VAC6Processor() override;

  /** Called at first after constructor */
  tresult PLUGIN_API initialize(FUnknown *context) override;

  /** Called at the end before destructor */
  tresult PLUGIN_API terminate() override;

  /** Switch the Plug-in on/off */
  tresult PLUGIN_API setActive(TBool state) override;

  /** Here we go...the process call */
  tresult PLUGIN_API process(ProcessData &data) override;

  /** Asks if a given sample size is supported see \ref SymbolicSampleSizes. */
  tresult PLUGIN_API canProcessSampleSize (int32 symbolicSampleSize) override;

  /** Restore the state (ex: after loading preset or project) */
  tresult PLUGIN_API setState(IBStream *state) override;

  /** Called to save the state (before saving a preset or project) */
  tresult PLUGIN_API getState(IBStream *state) override;

  //--- ---------------------------------------------------------------------
  // create function required for Plug-in factory,
  // it will be called to create new instances of this Plug-in
  //--- ---------------------------------------------------------------------
  static FUnknown *createInstance(void * /*context*/) { return (IAudioProcessor *) new VAC6Processor(); }

  tresult PLUGIN_API setupProcessing(ProcessSetup &setup) override;


protected:
  /**
   * Processes the parameters that have changed since the last call to process
   *
   * @param inputParameterChanges
   */
  void processParameters(IParameterChanges& inputParameterChanges);

  /**
   * Processes inputs (step 2 always called after processing the parameters)
   */
  tresult processInputs(ProcessData &data);

  /**
   * Processes inputs (step 2 always called after processing the parameters)
   */
  template<typename SampleType>
  tresult genericProcessInputs(ProcessData &data);

  template<typename SampleType>
  inline EMaxLevelState toMaxLevelState(SampleType value);

  // from ITimerCallback
  void onTimer(Timer *timer) override;

private:
  Timer *fTimer;
  MaxLevel fMaxLevel;
};

}
}
