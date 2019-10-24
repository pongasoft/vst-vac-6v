#pragma once

#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/vst/vsttypes.h>

namespace pongasoft::VST {

// generated with java/groovy UUID.randomUUID()

#ifndef NDEBUG
static const ::Steinberg::FUID VAC6ProcessorUID(0xe06dc4cc, 0xe518462a, 0xaedd6ccf, 0x0368c50e);
static const ::Steinberg::FUID VAC6ControllerUID(0xf9d99623, 0xb8784dc8, 0x94fe494f, 0xd5889d47);
#else
static const ::Steinberg::FUID VAC6ProcessorUID(0x3e9686b9, 0xcefb48f4, 0x8c85b178, 0xc6da18b6);
static const ::Steinberg::FUID VAC6ControllerUID(0x3415c383, 0x5e904dcf, 0xb6ccf5dd, 0x1d26cddb);
#endif

// tags associated to parameters
enum EVAC6ParamID : Steinberg::Vst::ParamID
{
  kBypass = 1000,

  kMaxLevelReset = 1010,
  kMaxLevelSinceResetMarker = 1030,
  kMaxLevelInWindowMarker = 1031,

  kSoftClippingLevel = 2000,

  kLCDZoomFactorX = 3010,   // zoom factor on the X axis (history)
  kLCDLeftChannel = 3020,   // toggle for showing/hiding left channel
  kLCDRightChannel = 3021,  // toggle for showing/hiding right channel
  kLCDLiveView = 3030,      // live view/pause toggle
  kLCDInputX = 3040,        // selected position on the screen when paused
  kLCDHistoryOffset = 3050, // position is a percent in the history [0.0, 1.0]

  kGain1 = 4000,
  kGain2 = 4010,
  kGainFilter = 4020,

  kHistoryData = 5000 // internal parameter used to communicate large amount of data between RT and GUI
};

// tags associated to custom views (not associated to params)
enum EVAC6CustomViewTag : Steinberg::Vst::ParamID
{
  kMaxLevelSinceReset = 1,
  kMaxLevelInWindow = 2,
  kMaxLevelForSelection = 3,
  kLCD = 4,
  KLCDScrollbar = 5,
  kGain = 6,
};

//------------------------------------------------------------------------
} // namespace pongasoft
