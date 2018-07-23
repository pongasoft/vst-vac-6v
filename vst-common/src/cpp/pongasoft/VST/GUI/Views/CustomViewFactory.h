#pragma once

#include <vstgui4/vstgui/uidescription/uiviewfactory.h>
#include <pongasoft/VST/GUI/Params/GUIParameters.h>

namespace pongasoft {
namespace VST {
namespace GUI {
namespace Views {

using namespace Params;

/**
 * interface to access vst parameters
 */
class GUIParametersProvider
{
public:
  virtual GUIParameters const &getGUIParameters() const = 0;
};

/**
 * Custom view factory to give access to vst parameters
 */
class CustomUIViewFactory : public VSTGUI::UIViewFactory, public GUIParametersProvider
{
public:
  explicit CustomUIViewFactory(GUIParameters const &iGUIParameters) : fGUIParameters{iGUIParameters}
  {
    DLOG_F(INFO, "CustomUIViewFactory()");
  }

  ~CustomUIViewFactory() override
  {
    DLOG_F(INFO, "~CustomUIViewFactory()");
  }

  GUIParameters const &getGUIParameters() const override
  {
    return fGUIParameters;
  }

private:
  GUIParameters const &fGUIParameters;
};


}
}
}
}