#pragma once

#include <vstgui4/vstgui/lib/cview.h>
#include <map>
#include "CustomViewCreator.h"
#include "VSTParameters.h"

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

class CustomView : public CView, RawParameter::IChangeListener
{
public:
  explicit CustomView(const CRect &size);

  // Deleting for now... not sure there is an actual use for it
  CustomView(const CustomView &c) = delete;

  void setBackColor(CColor const &color);
  CColor const &getBackColor() const { return fBackColor; }

  void setCustomViewTag (int32_t iTag) { fTag = iTag; }
  int32_t getCustomViewTag () const { return fTag; }

  void draw(CDrawContext *iContext) override;

  void drawStyleChanged();

  void onParameterChange(ParamID iParamID, ParamValue iNormalizedValue) override;

public:

  virtual void initParameters(std::shared_ptr<VSTParameters> iParameters)
  {
    fParameters = std::move(iParameters);
    registerParameters();
  }
  
  virtual void registerParameters()
  {
    // subclasses implements this method
  }
  
  std::unique_ptr<RawParameter> registerRawParameter(ParamID iParamID, bool iSubscribeToChanges = true);

  template<typename T>
  std::unique_ptr<T> registerVSTParameter(ParamID iParamID, bool iSubscribeToChanges = true)
  {
    return std::make_unique<T>(registerRawParameter(iParamID, iSubscribeToChanges));
  }

  std::unique_ptr<BooleanParameter> registerBooleanParameter(ParamID iParamID, bool iSubscribeToChanges = true);

  template<int StepCount>
  std::unique_ptr<DiscreteParameter<StepCount>> registerDiscreteParameter(ParamID iParamID, bool iSubscribeToChanges = true);

  CLASS_METHODS_NOCOPY(CustomView, CControl)

protected:
  int32_t fTag;
  CColor fBackColor;

  // Access to parameters
  std::shared_ptr<VSTParameters> fParameters;

  // Maintains the connections for the listeners... will be automatically discarded in the destructor
  std::map<ParamID, std::unique_ptr<RawParameter::Connection>> fParameterConnections;

public:
  class Creator : public CustomViewCreator<CustomView>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerTagAttribute("custom-view-tag", &CustomView::getCustomViewTag, &CustomView::setCustomViewTag);
      registerColorAttribute(UIViewCreator::kAttrBackColor, &CustomView::getBackColor, &CustomView::setBackColor);
    }
  };
};

///////////////////////////////////////////
// CustomView::registerDiscreteParameter
///////////////////////////////////////////
template<int StepCount>
std::unique_ptr<DiscreteParameter<StepCount>>
CustomView::registerDiscreteParameter(ParamID iParamID, bool iSubscribeToChanges)
{
  return registerVSTParameter<DiscreteParameter<StepCount>>(iParamID, iSubscribeToChanges);
}

}
}
}

