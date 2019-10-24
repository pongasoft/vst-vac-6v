#pragma once

#include <pongasoft/VST/GUI/Views/CustomView.h>
#include "../VAC6Model.h"
#include "../VAC6Plugin.h"

namespace pongasoft::VST::VAC6 {

using namespace VSTGUI;
using namespace Common;

using namespace GUI;
using namespace GUI::Views;
using namespace GUI::Params;

/**
 * Combine the 2 gains to display the total amount of gain */
class GainView : public StateAwareCustomView<VAC6GUIState>
{
public:
  // constructor
  explicit GainView(const CRect &iSize) : StateAwareCustomView<VAC6GUIState>{iSize} {};

  // get/setFontColor
  CColor const &getFontColor() const { return fFontColor; }
  void setFontColor(CColor const &iColor) { fFontColor = iColor; }

  // get/setFont
  FontPtr getFont() const { return fFont; }
  void setFont(FontPtr iFont) { fFont = iFont; }

public:
  // draw => does the actual drawing job
  void draw(CDrawContext *iContext) override;

  // registerParameters
  void registerParameters() override;

public:
  CLASS_METHODS_NOCOPY(GainView, CustomView)

protected:
  CColor fFontColor{kWhiteCColor};
  FontSPtr fFont{nullptr};

  GUIVstParam<Gain> fGain1Parameter{nullptr};
  GUIVstParam<Gain> fGain2Parameter{nullptr};
  GUIVstBooleanParam fBypassParameter{nullptr};

public:
  // Creator class
  class Creator : public CustomViewCreator<GainView, StateAwareCustomView<VAC6GUIState>>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerColorAttribute("font-color",
                             &GainView::getFontColor,
                             &GainView::setFontColor);
      registerFontAttribute("font",
                            &GainView::getFont,
                            &GainView::setFont);
    }
  };
};

}