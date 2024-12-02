
#include "VAC6CIDs.h"

#include "version.h"
#include "VAC6Processor.h"
#include "controller/VAC6Controller.h"

#include <pongasoft/VST/PluginFactory.h>

using namespace pongasoft::VST;

#ifndef NDEBUG
#define stringPluginName "VAC-6V_Debug"
#else
#define stringPluginName "VAC-6V"
#endif

//------------------------------------------------------------------------
//  VST3 Plugin Main entry point
//------------------------------------------------------------------------
EXPORT_FACTORY Steinberg::IPluginFactory* PLUGIN_API GetPluginFactory()
{
  return JambaPluginFactory::GetVST3PluginFactory<
    pongasoft::VST::VAC6::VAC6Processor, // processor class (Real Time)
    pongasoft::VST::VAC6::VAC6Controller // controller class (GUI)
  >("pongasoft", // company/vendor
    "https://www.pongasoft.com", // url
    "support@pongasoft.com", // email
    stringPluginName, // plugin name
    FULL_VERSION_STR, // plugin version
    Vst::PlugType::kFx // plugin category (can be changed to other like kInstrument, etc...)
  );
}
