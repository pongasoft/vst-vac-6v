#pragma once

#include <vstgui4/vstgui/lib/cview.h>
#include <map>
#include "CustomViewCreator.h"
#include "VSTParameters.h"

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;

/**
 * Base class that all custom views will inherit from. Defines a basic back color.
 * The custom view tag should be a tag associated to the view itself not a parameter (like it is the case for CControl).
 * The registerParameters method is the method that you inherit from to register which VST parameters your view will
 * use. By default each parameter will be also be registered to listen for changes which will trigger the view to be
 * redrawn: the onParameterChange method can be overridden to react differently (or additionally) to handle parameter changes.
 * You use the convenient registerXXParameter methods to register each parameter.
 * Ex:
 * ...
 * std::unique_ptr<BooleanParameter> fMyBoolParameter{nullptr};
 * std::unique_ptr<PercentParameter> fMyPercentParameter{nullptr};
 * std::unique_ptr<PercentParameter::Editor> fMyPercentParameterEditor{nullptr};
 * ...
 * void registerParameters() override
 * {
 *   fMyBoolParameter = registerBooleanParameter(EParamIDs::kMyBoolParamID);
 *   fMyPercentParameter = registerPercentParameter(EParamIDs::kMyPercentParamID);
 * }
 * ...
 * void draw() override
 * {
 *   CustomView::draw();
 *
 *   if(fMyBoolParameter->getValue())
 *     ... do something ...
 *
 *   CColor c = CColor{255, 255, 255, fMyPercentParameter->getValue() * 255);
 *   ...
 * }
 *
 * void onMouseDown(CPoint &where, const CButtonState &buttons)
 * {
 *    double percent = ...; // for example using where.y compute a percentage value
 *    fMyPercentParameterEditor = fMyPercentParameter->edit(percent);
 * }
 *
 * void onMouseMoved(CPoint &where, const CButtonState &buttons)
 * {
 *   if(fMyPercentParameterEditor)
 *   {
 *     double percent = ...; // for example using where.y compute a percentage value
 *     fMyPercentParameterEditor->setValue(percent);
 *   }
 * }
 *
 * void onMouseUp(CPoint &where, const CButtonState &buttons)
 * {
 *   if(fMyPercentParameterEditor)
 *   {
 *     double percent = ...; // for example using where.y compute a percentage value
 *     fMyPercentParameterEditor->commit(percent);
 *     fMyPercentParameterEditor = nullptr;
 *   }
 * }
 *
 * void onMouseCancel()
 * {
 *   if(fMyPercentParameterEditor)
 *   {
 *     fMyPercentParameterEditor->rollback();
 *     fMyPercentParameterEditor = nullptr;
 *   }
 * }
 */
class CustomView : public CView, RawParameter::IChangeListener
{
public:
  // Constructor
  explicit CustomView(const CRect &size);

  // Deleting for now... not sure there is an actual use for it
  CustomView(const CustomView &c) = delete;

  // setBackColor / getBackColor
  void setBackColor(CColor const &color);
  CColor const &getBackColor() const { return fBackColor; }

  // setCustomViewTag / getCustomViewTag
  void setCustomViewTag (int32_t iTag) { fTag = iTag; }
  int32_t getCustomViewTag () const { return fTag; }

  /**
   * The basic draw method which will erase the background with the back color. You override this method
   * to implement your additional own logic.
   */
  void draw(CDrawContext *iContext) override;

  /**
   * Called when the draw style is changed (simply marks the view dirty)
   */
  void drawStyleChanged();

  /**
   * Callback when a parameter changes. By default simply marks the view as dirty.
   */
  void onParameterChange(ParamID iParamID, ParamValue iNormalizedValue) override;

public:

  /**
   * Called by the controller to initialize the VSTParameters object
   */
  virtual void initParameters(std::shared_ptr<VSTParameters> iParameters)
  {
    fParameters = std::move(iParameters);
    registerParameters();
  }

  /**
   * Subclasses should override this method to register each parameter
   */
  virtual void registerParameters()
  {
    // subclasses implements this method
  }

  /**
   * Registers a raw parameter (no conversion)
   */
  std::unique_ptr<RawParameter> registerRawParameter(ParamID iParamID, bool iSubscribeToChanges = true);

  /**
   * Generic register with any kind of conversion
   */
  template<typename T>
  std::unique_ptr<T> registerVSTParameter(ParamID iParamID, bool iSubscribeToChanges = true)
  {
    return std::make_unique<T>(registerRawParameter(iParamID, iSubscribeToChanges));
  }

  // shortcut for BooleanParameter
  std::unique_ptr<BooleanParameter> registerBooleanParameter(ParamID iParamID, bool iSubscribeToChanges = true);

  // shortcut for PercentParameter
  std::unique_ptr<PercentParameter> registerPercentParameter(ParamID iParamID, bool iSubscribeToChanges = true);

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

}
}
}

