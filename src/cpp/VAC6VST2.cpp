#include "public.sdk/source/vst/vst2wrapper/vst2wrapper.h"
#include "VAC6CIDs.h"

//------------------------------------------------------------------------
::AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
  return Steinberg::Vst::Vst2Wrapper::create(GetPluginFactory(),
                                             pongasoft::VST::VAC6ProcessorUID,
                                             'TBDx',
                                             audioMaster);
}
