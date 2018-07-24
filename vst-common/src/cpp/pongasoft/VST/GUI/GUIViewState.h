#pragma once

#include <vstgui4/vstgui/lib/iviewlistener.h>
#include <pongasoft/logging/loguru.hpp>
#include <pongasoft/VST/GUI/Params/GUIParameters.h>
#include <pongasoft/VST/GUI/Params/GUIParamCxMgr.h>

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace VSTGUI;
using namespace Params;

/**
 * Encapsulates a view while managing its lifecyle: the idea is that an instance of this class is being owned by the
 * main controller (thus its lifespan is the entire life of the controller) but the view assigned to it can come and go
 * (as the user opens/closes the UI of the plugin)
 */
template<typename V>
class GUIViewState : public IViewListenerAdapter, public GUIRawParameter::IChangeListener
{
public:
  // Constructor
  GUIViewState() : fView{nullptr} {};

  // Called when the view was created and needs to be assigned to this instance
  void assign(V *view) {
    DCHECK_NOTNULL_F(view, "assign should not receive null pointer");

    if(fView)
    {
      fView->unregisterViewListener(this);
    }

    fView = view;
    fView->registerViewListener(this);

    afterAssign();
  }

  /**
   * An implementation will typically override this method to initialize the view depending on the state. It is called
   * after assigned as been done (thus fView != nullptr)
   */
  virtual void afterAssign() = 0;

  /**
   * Called prior to the view being deleted/unassigned in case some cleanup needs to happen */
  virtual void beforeUnassign()
  {
    fView->setState(nullptr);
  }

  /**
   * By default sets the view dirty
   */
  virtual void updateView()
  {
    if(fView)
      fView->setDirty(true);
  }

  /**
 * Called during initialization
 */
  virtual void initParameters(GUIParameters const &iParameters)
  {
    fParamCxMgr = iParameters.createParamCxMgr();
  }

  /**
   * Subclasses should override this method to register each parameter
   */
  virtual void registerParameters()
  {
    // subclasses implements this method
  }

  /**
   * Nothing to do by default... implements in derived classes */
  void onParameterChange(ParamID iParamID, ParamValue iNormalizedValue) override
  {
  }

  /**
 * Registers a raw parameter (no conversion)
 */
  std::unique_ptr<GUIRawParameter> registerGUIRawParam(ParamID iParamID, bool iSubscribeToChanges = true)
  {
    return fParamCxMgr->registerGUIRawParam(iParamID, iSubscribeToChanges ? this : nullptr);
  }

  /**
   * Generic register with any kind of conversion
   */
  template<typename ParamConverter>
  GUIParamUPtr<ParamConverter> registerGUIParam(ParamID iParamID, bool iSubscribeToChanges = true)
  {
    return std::make_unique<GUIParameter<ParamConverter>>(registerGUIRawParam(iParamID, iSubscribeToChanges));
  }

  /**
   * Generic register with any kind of conversion using an actual param def (no param id)
   */
  template<typename ParamConverter>
  GUIParamUPtr<ParamConverter> registerGUIParam(ParamDefSPtr<ParamConverter> iParamDef,
                                                bool iSubscribeToChanges = true)
  {
    return std::make_unique<GUIParameter<ParamConverter>>(registerGUIRawParam(iParamDef->fParamID, iSubscribeToChanges));
  }

  /**
   * Gives access to plugin parameters
   */
  template<typename TParameters>
  TParameters const *getPluginParameters() const
  {
    return fParamCxMgr->getPluginParameters<TParameters>();
  }

protected:
  /**
   * Callback that is called when the host is about to delete the view and as a result it needs to be unassigned from
   * this instance
   */
  void viewWillDelete(CView *view) override {
    DCHECK_EQ_F(view, fView, "should be called with the same object!");

    beforeUnassign();

    fView->unregisterViewListener(this);
    fView = nullptr;

  };

  V* fView;

  // Access to parameters
  std::unique_ptr<GUIParamCxMgr> fParamCxMgr;
};

/**
 * This subclass gives access to plugin parameters with the fParams variable (similar to PluginCustomView)
 */
template<typename V, typename TPluginParameters>
class PluginGUIViewState : public GUIViewState<V>
{
public:
  void initParameters(GUIParameters const &iParameters) override
  {
    GUIViewState<V>::initParameters(iParameters);
    if(GUIViewState<V>::fParamCxMgr)
      fParams = GUIViewState<V>::template getPluginParameters<TPluginParameters>();
  }

public:
  // direct access to parameters (ex: fParams->fBypassParam)
  TPluginParameters const *fParams{nullptr};
};

}
}
}