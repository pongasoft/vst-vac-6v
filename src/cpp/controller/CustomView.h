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
  explicit CustomView(const CRect &iSize);

  // Deleting for now... not sure there is an actual use for it
  CustomView(const CustomView &c) = delete;

  // setBackColor / getBackColor
  void setBackColor(CColor const &iColor);
  CColor const &getBackColor() const { return fBackColor; }

  // setCustomViewTag / getCustomViewTag
  void setCustomViewTag (int32_t iTag) { fTag = iTag; }
  int32_t getCustomViewTag () const { return fTag; }

  // setEditorMode / getEditorMode
  void setEditorMode(bool iEditorMode);
  bool getEditorMode() const;

  /**
   * Implement this if you want to have a specific behavior when editor mode is changed.
   */
#if EDITOR_MODE
  virtual void onEditorModeChanged() {}
#endif

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
  virtual void initParameters(std::shared_ptr<VSTParameters> const &iParameters)
  {
    fParameters = iParameters->createManager();
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
  bool fEditorMode;
  CColor fBackColor;

  // Access to parameters
  std::unique_ptr<VSTParametersManager> fParameters;

public:
  class Creator : public CustomViewCreator<CustomView>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerTagAttribute("custom-view-tag", &CustomView::getCustomViewTag, &CustomView::setCustomViewTag);
#if EDITOR_MODE
      registerBooleanAttribute("editor-mode", &CustomView::getEditorMode, &CustomView::setEditorMode);
#endif
      registerColorAttribute(UIViewCreator::kAttrBackColor, &CustomView::getBackColor, &CustomView::setBackColor);
    }
  };
};

/**
 * Base class for custom views providing one parameter only (similar to CControl)
 */
class CustomControlView : public CustomView
{
public:
  explicit CustomControlView(const CRect &iSize) : CustomView(iSize) {}

  // get/setControlTag
  void setControlTag (int32_t iTag);
  int32_t getControlTag () const { return fControlTag; }
  virtual void onControlTagChange() {}

public:
  CLASS_METHODS_NOCOPY(CustomControlView, CustomView)

protected:
  int32_t fControlTag{-1};

public:
  class Creator : public CustomViewCreator<CustomControlView>
  {
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreator(iViewName, iDisplayName)
    {
      registerAttributes(CustomView::Creator());
      registerTagAttribute("control-tag", &CustomControlView::getControlTag, &CustomControlView::setControlTag);
    }
  };
};

/**
 * Base class for custom views providing one parameter only (similar to CControl)
 * This base class automatically registers the custom control and also keeps a control value for the case when
 * the control does not exist (for example in editor the control tag may not be defined).
 */
template<typename TVSTParameter>
class TCustomControlView : public CustomControlView
{
public:
  // TCustomControlView
  explicit TCustomControlView(const CRect &iSize) : CustomControlView(iSize) {}

public:
  CLASS_METHODS_NOCOPY(CustomControlView, TCustomControlView)

  // set/getControlValue
  typename TVSTParameter::value_type getControlValue() const;
  void setControlValue(typename TVSTParameter::value_type const &iControlValue);

  // onControlTagChange
  void onControlTagChange() override;

  // registerParameters
  void registerParameters() override;

protected:
  // parameters are registered after the fact
  void registerControlParameter();

  // the vst parameter tied to the control
  std::unique_ptr<TVSTParameter> fControlParameter{nullptr};

#if EDITOR_MODE
  // the value (in sync with control parameter but may exist on its own in editor mode)
  typename TVSTParameter::value_type fControlValue{};
#endif

public:
  class Creator : public CustomViewCreator<TCustomControlView<TVSTParameter>>
  {
  private:
    using CustomViewCreatorT = CustomViewCreator<TCustomControlView<TVSTParameter>>;
  public:
    explicit Creator(char const *iViewName = nullptr, char const *iDisplayName = nullptr) :
      CustomViewCreatorT(iViewName, iDisplayName)
    {
      CustomViewCreatorT::registerAttributes(CustomControlView::Creator());
    }
  };
};

///////////////////////////////////////////
// TCustomControlView<TVSTParameter>::getControlValue
///////////////////////////////////////////
template<typename TVSTParameter>
typename TVSTParameter::value_type TCustomControlView<TVSTParameter>::getControlValue() const
{
#if EDITOR_MODE
  if(fControlParameter)
    return fControlParameter->getValue();
  else
    return fControlValue;
#else
  return fControlParameter->getValue();
#endif
}

///////////////////////////////////////////
// TCustomControlView<TVSTParameter>::setControlValue
///////////////////////////////////////////
template<typename TVSTParameter>
void TCustomControlView<TVSTParameter>::setControlValue(typename TVSTParameter::value_type const &iControlValue)
{
#if EDITOR_MODE
  fControlValue = iControlValue;
  if(fControlParameter)
    fControlParameter->setValue(fControlValue);
#else
  fControlParameter->setValue(iControlValue);
#endif
}

///////////////////////////////////////////
// TCustomControlView<TVSTParameter>::onControlTagChange
///////////////////////////////////////////
template<typename TVSTParameter>
void TCustomControlView<TVSTParameter>::onControlTagChange()
{
  CustomControlView::onControlTagChange();
  registerControlParameter();
}

///////////////////////////////////////////
// TCustomControlView<TVSTParameter>::registerParameters
///////////////////////////////////////////
template<typename TVSTParameter>
void TCustomControlView<TVSTParameter>::registerParameters()
{
  CustomControlView::registerParameters();
  registerControlParameter();
}

///////////////////////////////////////////
// TCustomControlView<TVSTParameter>::registerControlParameter
///////////////////////////////////////////
template<typename TVSTParameter>
void TCustomControlView<TVSTParameter>::registerControlParameter()
{
#if EDITOR_MODE
  if(getControlTag() >= 0)
  {
    auto paramID = static_cast<ParamID>(getControlTag());
    if(fParameters)
    {
      if(fParameters->exists(paramID))
      {
        fControlParameter = registerVSTParameter<TVSTParameter>(paramID);
        fControlValue = fControlParameter->getValue();
      }
      else
      {
        DLOG_F(WARNING, "Parameter[%d] does not exist", paramID);
        fControlParameter = nullptr;
      }
    }
  }
#else
  auto paramID = static_cast<ParamID>(getControlTag());
  if(fParameters)
  {
    if(fParameters->exists(paramID))
      fControlParameter = registerVSTParameter<TVSTParameter>(paramID);
    else
      ABORT_F("Could not find parameter for control tag [%d]", paramID);
  }
#endif
}

}
}
}

