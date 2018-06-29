#pragma once

#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/vst/vsttypes.h>

namespace pongasoft {
namespace VST {

// generated with java/groovy UUID.randomUUID()

static const ::Steinberg::FUID VAC6ProcessorUID(0x3e9686b9, 0xcefb48f4, 0x8c85b178, 0xc6da18b6);
static const ::Steinberg::FUID VAC6ControllerUID(0x3415c383, 0x5e904dcf, 0xb6ccf5dd, 0x1d26cddb);

// tags associated to parameters
enum EVAC6ParamID : Steinberg::Vst::ParamID
{
  kMaxLevelReset = 1010,
  kMaxLevelAutoReset = 1020,

  kSoftClippingLevel = 2000,

  kLCDZoomFactorX = 3010,   // zoom factor on the X axis (history)
  kLCDLeftChannel = 3020,   // toggle for showing/hiding left channel
  kLCDRightChannel = 3021,  // toggle for showing/hiding right channel
  kLCDLiveView = 3030,      // live view/pause toggle
  kLCDInputX = 3040,        // selected position on the screen when paused
  kLCDHistoryOffset = 3050, // position is a percent in the history [0.0, 1.0]
};

// tags associated to custom views (not associated to params)
enum EVAC6CustomViewTag : Steinberg::Vst::ParamID
{
  kMaxLevelValue = 1,
  kLCD = 2,
  KLCDScrollbar = 3
};

enum EVAC6MessageID
{
  kMaxLevel_MID = 100,
  kLCDData_MID = 200
};

//------------------------------------------------------------------------
} // namespace VST
} // namespace pongasoft
