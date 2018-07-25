#pragma once

#include <base/source/timer.h>
#include <public.sdk/source/vst/vstaudioeffect.h>
#include <pongasoft/VST/Timer.h>
#include "RTState.h"

namespace pongasoft {
namespace VST {
namespace RT {

using namespace Steinberg;
using namespace Steinberg::Vst;

class RTProcessor : public AudioEffect
{
public:
  RTProcessor(Steinberg::FUID const &iControllerUID);

  ~RTProcessor() override = default;

  virtual IRTState *getRTState() = 0;

  /** Switch the Plug-in on/off */
  tresult PLUGIN_API setActive(TBool state) override;

  /** Here we go...the process call */
  tresult PLUGIN_API process(ProcessData &data) override;

  /** Asks if a given sample size is supported see \ref SymbolicSampleSizes. */
  tresult PLUGIN_API canProcessSampleSize(int32 symbolicSampleSize) override;

  /** Restore the state (ex: after loading preset or project) */
  tresult PLUGIN_API setState(IBStream *state) override;

  /** Called to save the state (before saving a preset or project) */
  tresult PLUGIN_API getState(IBStream *state) override;

protected:

  /**
   * @return true if you can handle 32 bits (true buy default) */
  virtual bool canProcess32Bits() const { return true; }

  /**
   * @return true if you can handle 64 bits (true buy default) */
  virtual bool canProcess64Bits() const { return true; }

  /**
   * Processes inputs (step 2 always called after processing the parameters)
   * Delegate to processInputs32Bits or processInputs64Bits accordingly
   */
  virtual tresult processInputs(ProcessData &data);

  /**
   * Processes inputs (step 2 always called after processing the parameters) for 32 bits
   */
  virtual tresult processInputs32Bits(ProcessData &data) { return kResultOk; }

  /**
   * Processes inputs (step 2 always called after processing the parameters) for 64 bits
   */
  virtual tresult processInputs64Bits(ProcessData &data) { return kResultOk; }

  /**
   * Subclass will implement this method to respond to the GUI timer firing
   * /////// WARNING !!!!! WARNING !!!!! WARNING !!!!! WARNING !!!!! WARNING !!!!! WARNING !!!!! //////
   * /////// this method WILL be called from the UI thread so do not modify the processor thread //////
   * /////// WARNING !!!!! WARNING !!!!! WARNING !!!!! WARNING !!!!! WARNING !!!!! WARNING !!!!!  //////
   * */
  virtual void onGUITimer() {}

  /**
   * Call this method to enable the GUI timer (onGUITimer will be called at the specified frequency)
   * Should be called in the constructor or setupProcessing method as it will take effect in setActive
   */
  void enableGUITimer(uint32 iUIFrameRateMs);

private:
  template<typename T>
  class GUITimerCallback : public ITimerCallback
  {
  public:
    GUITimerCallback(T *iTarget) : fTarget{iTarget} {}

    void onTimer(Timer *timer) override
    {
      fTarget->onGUITimer();
    }
  private:
    T *fTarget;
  };

private:
  GUITimerCallback<RTProcessor> fGUITimerCallback;

  uint32 fGUITimerIntervalMs;
  std::unique_ptr<AutoReleaseTimer> fGUITimer;

  bool fActive;

  int32 fSymbolicSampleSize = -1;
};

}
}
}
