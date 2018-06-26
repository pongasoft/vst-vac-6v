#pragma once

#include <public.sdk/source/vst/vstparameters.h>
#include <public.sdk/source/vst/vsteditcontroller.h>
#include "../logging/loguru.hpp"
#include "../Parameter.h"

namespace pongasoft {
namespace VST {
namespace GUI {

using namespace Steinberg;
using namespace Steinberg::Vst;

/**
 * Encapsulates access to the vst parameters defined in the controller
 */
class VSTParameters
{
public:
  /**
   * Interface to implement to receive parameter changes
   */
  class IChangeListener
  {
  public:
    virtual void onParameterChange(ParamID iParamID, ParamValue iNormalizedValue) = 0;
  };

  /**
   * Wrapper to edit a single parameter. Usage:
   *
   * // from a CView::onMouseDown callback
   * fMyParamEditor = fParameters.edit(myParamID);
   * fParamEditor->setNormalizedValue(myValue);
   *
   * // from a CView::onMouseMoved callback
   * fParamEditor->setNormalizedValue(myValue);
   *
   * // from a CView::onMouseUp/onMouseCancelled callback
   * fMyParamEditor->commit();
   */
  class Editor
  {
  public:
    inline Editor(ParamID iParamID, EditController *iController) :
      fParamID{iParamID},
      fController{iController}
    {
      DLOG_F(INFO, "VSTParameters::Editor(%d)", fParamID);
      fIsEditing = fController->beginEdit(fParamID) == kResultOk;
      fInitialParamValue = fController->getParamNormalized(fParamID);
    }

    Editor(Editor const &) = delete;
    Editor& operator=(Editor const &) = delete;

    /**
     * Change the value of the parameter. Note that nothing happens if you have called commit or rollback already
     */
    inline tresult setNormalizedValue(ParamValue iValue)
    {
      if(fIsEditing)
      {
        if(fController->setParamNormalized(fParamID, iValue) == kResultOk)
          return fController->performEdit(fParamID, fController->getParamNormalized(fParamID));
      }
      return kResultFalse;
    }

    /**
     * Shortcut when you want to set a boolean value
     */
    inline tresult setBooleanValue(bool iValue)
    {
      return setNormalizedValue(Common::normalizeBoolValue(iValue));
    }

    /*
     * Call when you are done with the modifications.
     * This has no effect if rollback() has already been called
     */
    inline tresult commit()
    {
      if(fIsEditing)
      {
        fIsEditing = false;
        return fController->endEdit(fParamID);
      }
      return kResultFalse;
    }

    /**
     * Call this if you want to revert to the original value of the parameter (when the editor is created).
     * This has no effect if commit() has already been called
     */
    inline tresult rollback()
    {
      if(fIsEditing)
      {
        setNormalizedValue(fInitialParamValue);
        fIsEditing = false;
        return fController->endEdit(fParamID);
      }
      return kResultFalse;
    }

    /**
     * Destructor which calls rollback by default
     */
    inline ~Editor()
    {
      DLOG_F(INFO, "~VSTParameter::Editor(%d)", fParamID);
      rollback();
    }

  private:
    ParamID fParamID;
    EditController *const fController;

    ParamValue fInitialParamValue;
    bool fIsEditing;
  };

  /**
   * Wrapper class which maintains the connection between a parameter and its listener. The connection will be
   * terminated if close() is called or automatically when the destructor is called.
   */
  class Connection : public Steinberg::FObject
  {
  public:
    inline Connection(ParamID iParamID, EditController *iController, IChangeListener *iChangeListener) :
      fParamID{iParamID},
      fController{iController},
      fChangeListener{iChangeListener}
    {
      DLOG_F(INFO, "VSTParameters::Connection(%d)", fParamID);

      DCHECK_NOTNULL_F(fParameter);

      fParameter = fController->getParameterObject(fParamID);

      DCHECK_NOTNULL_F(fChangeListener);

      fParameter->addRef();
      fParameter->addDependent(this);
      fIsConnected = true;
    }

    /**
     * Call to stop listening for changes. Also called automatically from the destructor.
     */
    inline void close()
    {
      if(fIsConnected)
      {
        fParameter->removeDependent(this);
        fParameter->release();
        fIsConnected = false;
      }
    }

    /**
     * Automatically closes the connection and stops listening */
    inline ~Connection() override
    {
      DLOG_F(INFO, "~VSTParameter::Connection(%d)", fParamID);
      close();
    }

    void PLUGIN_API update(FUnknown *iChangedUnknown, Steinberg::int32 iMessage) SMTG_OVERRIDE
    {
      if(iMessage == IDependent::kChanged)
      {
        fChangeListener->onParameterChange(fParamID, fController->getParamNormalized(fParamID));
      }
    }

    Connection(Editor const &) = delete;
    Connection& operator=(Editor const &) = delete;

  private:
    ParamID fParamID;
    Parameter *fParameter;
    EditController *const fController;
    IChangeListener *const fChangeListener;
    bool fIsConnected;
  };

public:
  // Constructor
  explicit VSTParameters(EditController *iController) :
    fController{iController}
  {
    DCHECK_NOTNULL_F(iController);
    DLOG_F(INFO, "VSTParameters::VSTParameters()");
  };

  VSTParameters(VSTParameters const&) = delete;
  VSTParameters& operator=(VSTParameters const &) = delete;

  // Destructor
  ~VSTParameters()
  {
    DLOG_F(INFO, "~VSTParameters::VSTParameters()");
  }

  /**
   * @return the normalized value of the given parameter id
   */
  ParamValue getNormalizedValue(ParamID iParamID) const
  {
    return fController->getParamNormalized(iParamID);
  }

  /**
   * Sets the normalized value to the give parameter id. Note that this is transactional and if you want to make
   * further changes that spans multiple calls (ex: onMouseDown / onMouseMoved / onMouseUp) you should use an editor
   */
  tresult setNormalizedValue(ParamID iParamID, ParamValue iValue)
  {
    Editor editor(iParamID, fController);
    editor.setNormalizedValue(iValue);
    return editor.commit();
  }

  /**
   * @return the value of the given parameter id as a boolean (does denormalization automatically)
   */
  bool getBooleanValue(ParamID iParamID) const
  {
    return Common::denormalizeBoolValue(getNormalizedValue(iParamID));
  }

  /**
   * Automatically normalize the parameter from a boolean
   */
  tresult setBooleanValue(ParamID iParamID, bool iValue)
  {
    return setNormalizedValue(iParamID, Common::normalizeBoolValue(iValue));
  }

  /**
   * @return the parameter as a discrete value
   */
  template<typename T>
  T getDiscreteValue(ParamID iParamID, T iStepCount) const
  {
    return Common::denormalizeDiscreteValue(iStepCount, getNormalizedValue(iParamID));
  }

  /**
   * Automatically normalize the parameter from a discrete value
   */
  tresult setDiscreteValue(ParamID iParamID, int iStepCount, int iDiscreteValue)
  {
    return setNormalizedValue(iParamID, Common::normalizeDiscreteValue(iStepCount, iDiscreteValue));
  }

  /**
   * @return and editor to modify the parameter (see Editor)
   */
  std::unique_ptr<Editor> edit(ParamID iParamID)
  {
    return std::make_unique<Editor>(iParamID, fController);
  }

  /**
   * @return a connection that will listen to parameter changes (see Connection)
   */
  std::unique_ptr<Connection> connect(ParamID iParamID, IChangeListener *iChangeListener)
  {
    return std::make_unique<Connection>(iParamID, fController, iChangeListener);
  }


private:
  EditController *const fController;
};

}
}
}