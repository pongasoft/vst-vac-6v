#pragma once

#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include "AudioUtils.h"

namespace pongasoft {
namespace VST {
namespace Common {

using namespace Steinberg;
using namespace Steinberg::Vst;

template<typename SampleType>
class AudioBuffers
{
public:
  typedef AudioBuffers<SampleType> class_type;

public:
  AudioBuffers(AudioBusBuffers &buffer, int32 numSamples) : fBuffer(buffer), fNumSamples(numSamples)
  {}

  // returns true if the buffer is silent (meaning all channels are silent => set to 1)
  inline bool isSilent() const
  {
    return fBuffer.silenceFlags == (static_cast<uint64>(1) << fBuffer.numChannels) - 1;
  }

  /**
   * Computes and adjust the silence flags
   * @return true if the buffer is silent (meaning all channels are silent)
   */
  inline bool adjustSilenceFlags()
  {
    uint64 silenceFlags = 0;

    auto buffer = getBuffer();

    for(int32 channel = 0; channel < getNumChannels(); channel++)
    {
      bool silent = true;

      auto ptr = buffer[channel];

      for(int j = 0; j < getNumSamples(); ++j, ptr++)
      {
        auto sample = *ptr;

        if(silent && !pongasoft::VST::Common::isSilent(sample))
          silent = false;
      }

      if(silent)
        silenceFlags |= static_cast<uint64>(1) << channel;
    }

    fBuffer.silenceFlags = silenceFlags;

    return isSilent();
  }

  // returns the underlying buffer
  inline SampleType **getBuffer() const;

  /**
   * @return number of channels (2 for stereo) of the underlying buffer
   */
  inline int32 getNumChannels() const { return fBuffer.numChannels; }

  /**
   * @return number of samples in the buffer
   */
  inline int32 getNumSamples() const { return fNumSamples; }

  /**
   * Copy the content of THIS buffer to the provided buffer (up to num samples)
   */
  inline tresult copyTo(class_type &toBuffer) const { return toBuffer.copyFrom(*this); };

  /**
   * Copy the content of the provided buffer to THIS buffer (up to num samples)
   */
  tresult copyFrom(class_type const &fromBuffer)
  {
    SampleType **fromSamples = fromBuffer.getBuffer();
    SampleType **toSamples = getBuffer();

    // there are cases when the 2 buffers could be identical.. no need to copy
    if(fromSamples == toSamples)
      return kResultOk;

    int32 numChannels = std::min(getNumChannels(), fromBuffer.getNumChannels());
    int32 numSamples = std::min(getNumSamples(), fromBuffer.getNumSamples());

    for(int32 channel = 0; channel < numChannels; channel++)
    {
      auto ptrFrom = fromSamples[channel];
      auto ptrTo = toSamples[channel];

      for(int i = 0; i < numSamples; ++i, ptrFrom++, ptrTo++)
      {
        *ptrTo = *ptrFrom;
      }
    }

    return kResultOk;
  }

  /**
   * @return the max sample (absolute) across all channels
   */
  inline SampleType absoluteMax()
  {
    SampleType max = 0;

    auto buffer = getBuffer();

    for(int32 channel = 0; channel < getNumChannels(); channel++)
    {
      auto ptr = buffer[channel];

      for(int j = 0; j < getNumSamples(); ++j, ptr++)
      {
        auto sample = *ptr;
        if(sample < 0)
          sample -= sample;

        max = std::max(max, sample);
      }
    }

    return max;
  }

private:
  AudioBusBuffers &fBuffer;
  const int32 fNumSamples;
};

template<>
inline Sample32 **AudioBuffers<Sample32>::getBuffer() const
{ return fBuffer.channelBuffers32; }

template<>
inline Sample64 **AudioBuffers<Sample64>::getBuffer() const
{ return fBuffer.channelBuffers64; }

typedef AudioBuffers<Sample32> AudioBuffers32;
typedef AudioBuffers<Sample64> AudioBuffers64;

}
}
}