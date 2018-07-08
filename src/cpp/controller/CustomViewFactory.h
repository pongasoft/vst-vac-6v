#pragma once

#include "VSTParameters.h"
#include <vstgui4/vstgui/uidescription/uiviewfactory.h>

namespace pongasoft {
namespace VST {
namespace GUI {

/**
 * interface to access vst parameters
 */
class VSTParametersProvider
{
public:
  virtual std::shared_ptr<VSTParameters> getVSTParameters() const = 0;
};

/**
 * Custom view factory to give access to vst parameters
 */
class CustomUIViewFactory : public VSTGUI::UIViewFactory, public VSTParametersProvider
{
public:
  explicit CustomUIViewFactory(std::shared_ptr<VSTParameters> iVSTParameters) : fVSTParameters{std::move(iVSTParameters)}
  {
    DLOG_F(INFO, "CustomUIViewFactory()");
  }

  ~CustomUIViewFactory() override
  {
    DLOG_F(INFO, "~CustomUIViewFactory()");
  }

  std::shared_ptr<VSTParameters> getVSTParameters() const override
  {
    return fVSTParameters;
  }

private:
  std::shared_ptr<VSTParameters> fVSTParameters;
};


}
}
}