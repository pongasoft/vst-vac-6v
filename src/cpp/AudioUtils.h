#pragma once

#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <cmath>
#include <pongasoft/logging/loguru.hpp>

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

/**
 * Make sure that the value remains within its bounds
 */
template <typename T, typename U>
inline static T clamp(const U &value, const T &lower, const T &upper)
{
  auto v = static_cast<T>(value);
  return v < lower ? lower : (v > upper ? upper : value);
}

/**
 * Same as clamp except it will actually fail/assert in debug mode. For example can be used to
 * access an array with an index and making sure the index is valid within the array. If it happens in production
 * release then it will no randomly crash the application by accessing random memory.
 */
template <typename T, typename U>
inline static T clampE(const U &value, const T &lower, const T &upper)
{
  auto v = static_cast<T>(value);
  DCHECK_F(v >= lower && v <= upper);
  return v < lower ? lower : (v > upper ? upper : value);
}

/**
 * Equals with 5 digits precision */
template <typename T>
inline static bool equals5DP(T const &v1, T const &v2)
{
  return std::fabs(v1 - v2) < 1e-5;
}

}
}
}
