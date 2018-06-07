#pragma once

#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/vst/vsttypes.h>

namespace pongasoft {
namespace VST {

// generated with java/groovy UUID.randomUUID()
  
static const ::Steinberg::FUID VAC6ProcessorUID(0x3e9686b9, 0xcefb48f4, 0x8c85b178, 0xc6da18b6);
static const ::Steinberg::FUID VAC6ControllerUID(0x3415c383, 0x5e904dcf, 0xb6ccf5dd, 0x1d26cddb);

enum EVAC6ParamID : Steinberg::Vst::ParamID {

  kMaxLevelValue = 1000,

  kSoftClippingLevel = 2000
};

enum EVAC6MessageID
{
  kMaxLevel_MID = 100
};

//------------------------------------------------------------------------
} // namespace VST
} // namespace pongasoft
