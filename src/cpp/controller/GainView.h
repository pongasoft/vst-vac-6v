#pragma once

#include "../VAC6Model.h"
#include "CustomView.h"

namespace pongasoft {
namespace VST {
namespace VAC6 {

using namespace VSTGUI;
using namespace Common;
using namespace GUI;

//using GainParameter = VSTParameterFromClass<Gain>; // CLion unhappy :(

using GainParameter = VSTParameter<Gain, Gain::denormalize, Gain::normalize>;

/**
 * Combine the 2 gains to display the total amount of gain */
class GainView : public CustomView
{
public:
  // constructor
  explicit GainView(const CRect &iSize) : CustomView{iSize} {};

  // get/setFontColor
  CColor const &getFontColor() const { return fFontColor; }
  void setFontColor(CColor const &iColor) { fFontColor = iColor; }

public:
  // draw => does the actual drawing job
  void draw(CDrawContext *iContext) override;

  // registerParameters
  void registerParameters() override;

public:
  CLASS_METHODS_NOCOPY(GainView, CustomView)

protected:
  CColor fFontColor{kWhiteCColor};
  std::unique_ptr<GainParameter> fGain1Parameter{nullptr};
  std::unique_ptr<GainParameter> fGain2Parameter{nullptr};

public:
  // Creator class
  class Creator : public CustomViewCreator<GainView, CustomView>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerColorAttribute("font-color",
                             &GainView::getFontColor,
                             &GainView::setFontColor);
    }
  };
};

}
}
}