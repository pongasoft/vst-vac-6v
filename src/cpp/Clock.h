#pragma once

#include <pluginterfaces/vst/vsttypes.h>
#include <cmath>

namespace pongasoft {
namespace VST {
namespace Common {

using namespace Steinberg::Vst;


class SampleRateBasedClock
{
public:
  /**
   * Keeps track of the time in number of samples processed vs sample rate
   */
  class RateLimiter
  {
  public:
    explicit RateLimiter(long iRateLimitInSamples = 0) : fRateLimitInSamples{iRateLimitInSamples}, fSampleCount{0}
    {}

    /**
     * Calls this method when a new batch of samples is processed and returns true if the limit (in samples) is
     * achieved
     */
    bool shouldUpdate(int numSamples)
    {
      fSampleCount += numSamples;
      if(fSampleCount >= fRateLimitInSamples)
      {
        fSampleCount -= fRateLimitInSamples;
        return true;
      }
      return false;
    }

  private:
    long fRateLimitInSamples;
    long fSampleCount;
  };

  explicit SampleRateBasedClock(SampleRate iSampleRate) : fSampleRate{iSampleRate}
  {

  }

  long getSampleCountFor(long iMillis) const
  {
    return static_cast<long>(ceil(fSampleRate * iMillis / 1000.0));
  }

  SampleRate getSampleRate() const
  {
    return fSampleRate;
  }

  void setSampleRate(SampleRate iSampleRate)
  {
    fSampleRate = iSampleRate;
  }

  RateLimiter getRateLimiter(long iMillis) const
  {
    return RateLimiter{getSampleCountFor(iMillis)};
  }

private:
  SampleRate fSampleRate;
};

}
}
}
