#pragma once

#include <pluginterfaces/vst/ivstaudioprocessor.h>

namespace pongasoft {
namespace VST {
namespace Common {

using namespace Steinberg;
using namespace Steinberg::Vst;

#define BIT_SET(a,b) ((a) |= (static_cast<uint64>(1)<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(static_cast<uint64>(1)<<(b)))

// defines the threshold of silence
constexpr Sample32 Sample32SilentThreshold = ((Sample32)2.0e-8);
constexpr Sample64 Sample64SilentThreshold = ((Sample64)2.0e-8);

// check if sample is silent (lower than threshold) Sample32 version
inline bool isSilent(Sample32 value)
{
  if(value < 0)
    value = -value;
  return value <= Sample32SilentThreshold;
}

// check if sample is silent (lower than threshold) Sample64 version
inline bool isSilent(Sample64 value)
{
  if(value < 0)
    value = -value;
  return value <= Sample64SilentThreshold;
}

template <typename T>
inline static const T& clamp(const T &value, const T &lower, const T &upper)
{
  return value < lower ? lower : (value > upper ? upper : value);
}

/**
 * Equals with 5 digits precision */
template <typename T>
inline static bool equals5DP(T const &v1, T const &v2)
{
  return std::fabs(v1 - v2) < 1e-5;
}

class ClockSample
{
public:
  ClockSample(SampleRate iSampleRate) : fSampleRate(iSampleRate) {}

private:
  SampleRate fSampleRate;
};

//// returns true if the buffer is silent (meaning all channels are silent => set to 1)
//inline bool isSilent(AudioBusBuffers &buffer)
//{
//  return buffer.silenceFlags == (static_cast<uint64>(1) << buffer.numChannels) - 1;
//}
//
///**
// * Use of template to retrieve the proper buffer
// */
//template<typename SampleType>
//inline SampleType** getBuffer(AudioBusBuffers &buffer);
//
//// specialization for Sample32
//template<>
//inline Sample32** getBuffer(AudioBusBuffers &buffer) { return buffer.channelBuffers32; }
//
//// specialization for Sample64
//template<>
//inline Sample64** getBuffer(AudioBusBuffers &buffer) { return buffer.channelBuffers64; }

}
}
}
