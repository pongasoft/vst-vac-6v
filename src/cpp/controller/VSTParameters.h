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

// making sure we can easily change this type
using ParameterOwner = EditController;

///////////////////////////////////////////
// RawParameter
///////////////////////////////////////////

/**
 * Encapsulates a vst parameter and how to access it (read/write) as well as how to "connect" to it in order to be
 * notified of changes. This "raw" version deals with ParamValue which is the underlying type used by the vst sdk
 * which is always a number in the range [0.0, 1.0]. The class VSTParameter deals with other types and automatic
 * normalization/denormalization.
 */
class RawParameter
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

public:
  /**
   * Wrapper to edit a single parameter. Usage:
   *
   * // from a CView::onMouseDown callback
   * fMyParamEditor = fParameter.edit(myParamID);
   * fParamEditor->setValue(myValue);
   *
   * // from a CView::onMouseMoved callback
   * fParamEditor->setValue(myValue);
   *
   * // from a CView::onMouseUp/onMouseCancelled callback
   * fMyParamEditor->commit();
   */
  class Editor
  {
  public:
    inline Editor(ParamID iParamID, ParameterOwner *iParameterOwner) :
      fParamID{iParamID},
      fParameterOwner{iParameterOwner}
    {
      DLOG_F(INFO, "RawParameter::Editor(%d)", fParamID);
      fParameterOwner->beginEdit(fParamID);
      fIsEditing = true;
      fInitialParamValue = fParameterOwner->getParamNormalized(fParamID);
    }

    // disabling copy
    Editor(Editor const &) = delete;
    Editor& operator=(Editor const &) = delete;

    /**
     * Change the value of the parameter. Note that nothing happens if you have called commit or rollback already
     */
    inline tresult setValue(ParamValue iValue)
    {
      tresult res = kResultFalse;
      if(fIsEditing)
      {
        res = fParameterOwner->setParamNormalized(fParamID, iValue);
        if(res == kResultOk)
          fParameterOwner->performEdit(fParamID, fParameterOwner->getParamNormalized(fParamID));
      }
      return res;
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
        fParameterOwner->endEdit(fParamID);
        return kResultOk;
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
        setValue(fInitialParamValue);
        fIsEditing = false;
        fParameterOwner->endEdit(fParamID);
        return kResultOk;
      }
      return kResultFalse;
    }

    /**
     * Destructor which calls rollback by default
     */
    inline ~Editor()
    {
      DLOG_F(INFO, "~RawParameter::Editor(%d)", fParamID);
      rollback();
    }

  private:
    ParamID fParamID;
    ParameterOwner *const fParameterOwner;

    ParamValue fInitialParamValue;
    bool fIsEditing;
  };

public:
  /**
   * Wrapper class which maintains the connection between a parameter and its listener. The connection will be
   * terminated if close() is called or automatically when the destructor is called.
   */
  class Connection : public Steinberg::FObject
  {
  public:
    inline Connection(ParamID iParamID, ParameterOwner *iParameterOwner, IChangeListener *iChangeListener) :
      fParamID{iParamID},
      fParameterOwner{iParameterOwner},
      fChangeListener{iChangeListener}
    {
      DLOG_F(INFO, "RawParameter::Connection(%d)", fParamID);

      DCHECK_NOTNULL_F(fParameterOwner);

      fParameter = fParameterOwner->getParameterObject(fParamID);

      DCHECK_NOTNULL_F(fParameter);
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
      DLOG_F(INFO, "~RawParameter::Connection(%d)", fParamID);
      close();
    }

    /**
     * This is being called when the parameter receives a message... do not call explicitely
     */
    void PLUGIN_API update(FUnknown *iChangedUnknown, Steinberg::int32 iMessage) SMTG_OVERRIDE
    {
      if(iMessage == IDependent::kChanged)
      {
        fChangeListener->onParameterChange(fParamID, fParameterOwner->getParamNormalized(fParamID));
      }
    }

    // disabling copy
    Connection(Editor const &) = delete;
    Connection& operator=(Editor const &) = delete;

  private:
    ParamID fParamID;
    Parameter *fParameter;
    ParameterOwner *const fParameterOwner;
    IChangeListener *const fChangeListener;
    bool fIsConnected;
  };

public:
  // Constructor
  RawParameter(ParamID iParamID, ParameterOwner *iParameterOwner) :
    fParamID{iParamID},
    fParameterOwner{iParameterOwner}
  {
    DLOG_F(INFO, "RawParameter::RawParameter(%d)", fParamID);
    DCHECK_NOTNULL_F(fParameterOwner);

    fParameter = fParameterOwner->getParameterObject(fParamID);
    DCHECK_NOTNULL_F(fParameter);
  }

  // Destructor
  ~RawParameter()
  {
    DLOG_F(INFO, "RawParameter::~RawParameter(%d)", fParamID);
  }

  // getParamID
  ParamID getParamID() const
  {
    return fParamID;
  }

  /**
   * @return the current raw value of the parameter
   */
  ParamValue getValue() const
  {
    return fParameterOwner->getParamNormalized(fParamID);
  }

  /**
   * Sets the value of this parameter. Note that this is "transactional" and if you want to make
   * further changes that spans multiple calls (ex: onMouseDown / onMouseMoved / onMouseUp) you should use an editor
   */
  tresult setValue(ParamValue iValue)
  {
    Editor editor(fParamID, fParameterOwner);
    editor.setValue(iValue);
    return editor.commit();
  }

  /**
   * @return an editor to modify the parameter (see Editor)
   */
  std::unique_ptr<Editor> edit()
  {
    return std::make_unique<Editor>(fParamID, fParameterOwner);
  }

  /**
   * @return a connection that will listen to parameter changes (see Connection)
   */
  std::unique_ptr<Connection> connect(IChangeListener *iChangeListener)
  {
    return std::make_unique<Connection>(fParamID, fParameterOwner, iChangeListener);
  }

private:
  ParamID fParamID;
  Parameter *fParameter;
  ParameterOwner *const fParameterOwner;
};

///////////////////////////////////////////
// VSTParameter
///////////////////////////////////////////

// the function pointer type to denormalize a raw value
template<typename T>
using VSTParameterDenormalizer = T (*)(ParamValue);

// the function pointer type to normalize a raw value
template<typename T>
using VSTParameterNormalizer = ParamValue (*)(T const &);

/**
 * This class wraps a RawParameter to deal with any type using a Normalizer and Denormalizer functions
 */
template<typename T, VSTParameterDenormalizer<T> Denormalizer, VSTParameterNormalizer<T> Normalizer>
class VSTParameter
{
public:
  typedef T value_type;
  typedef VSTParameter<T, Denormalizer, Normalizer> class_type;

public:
  /**
   * Wrapper to edit a single parameter. Usage:
   *
   * // from a CView::onMouseDown callback
   * fMyParamEditor = fParameter.edit(myParamID);
   * fParamEditor->setValue(myValue);
   *
   * // from a CView::onMouseMoved callback
   * fParamEditor->setValue(myValue);
   *
   * // from a CView::onMouseUp/onMouseCancelled callback
   * fMyParamEditor->commit();
   */
  class Editor
  {
  public:
    inline explicit Editor(std::unique_ptr<RawParameter::Editor> iRawEditor) :
      fRawEditor{std::move(iRawEditor)}
    {
    }

    // disabling copy
    Editor(Editor const &) = delete;
    Editor& operator=(Editor const &) = delete;

    /**
     * Change the value of the parameter. Note that nothing happens if you have called commit or rollback already
     */
    inline tresult setValue(T iValue)
    {
      return fRawEditor->setValue(Normalizer(iValue));
    }

    /*
     * Call when you are done with the modifications.
     * This has no effect if rollback() has already been called
     */
    inline tresult commit()
    {
      return fRawEditor->commit();
    }

    /*
     * Shortcut to set the value prior to commit
     * Call when you are done with the modifications.
     * This has no effect if rollback() has already been called
     */
    inline tresult commit(T iValue)
    {
      setValue(iValue);
      return commit();
    }

    /**
     * Call this if you want to revert to the original value of the parameter (when the editor is created).
     * This has no effect if commit() has already been called
     */
    inline tresult rollback()
    {
      return fRawEditor->rollback();
    }

  private:
    std::unique_ptr<RawParameter::Editor> fRawEditor;
  };

public:
  // Constructor
  explicit VSTParameter(std::unique_ptr<RawParameter> iRawParameter) :
    fRawParameter{std::move(iRawParameter)}
  {
    DCHECK_NOTNULL_F(fRawParameter.get());
    DLOG_F(INFO, "VSTParameter::VSTParameter(%d)", fRawParameter->getParamID());
  }

  // Destructor
  ~VSTParameter()
  {
    DLOG_F(INFO, "VSTParameter::~VSTParameter(%d)", fRawParameter->getParamID());
  }

  // getParamID
  ParamID getParamID() const
  {
    return fRawParameter->getParamID();
  }

  /**
   * @return the current value of the parameter as a T (using the Denormalizer)
   */
  T getValue() const
  {
    return Denormalizer(fRawParameter->getValue());
  }

  /**
   * Sets the value of this parameter. Note that this is "transactional" and if you want to make
   * further changes that spans multiple calls (ex: onMouseDown / onMouseMoved / onMouseUp) you should use an editor
   */
  tresult setValue(T iValue)
  {
    return fRawParameter->setValue(Normalizer(iValue));
  }

  /**
   * @return an editor to modify the parameter (see Editor)
   */
  std::unique_ptr<Editor> edit()
  {
    return std::make_unique<Editor>(fRawParameter->edit());
  }

  /**
   * Shortcut to create an editor and set the value to it
   *
   * @return an editor to modify the parameter (see Editor)
   */
  std::unique_ptr<Editor> edit(T iValue)
  {
    auto editor = std::make_unique<Editor>(fRawParameter->edit());
    editor->setValue(iValue);
    return editor;
  }

private:
  std::unique_ptr<RawParameter> fRawParameter;
};

/**
 * Encapsulates access to the vst parameters defined in the controller
 */
class VSTParameters
{
public:
  // Constructor
  explicit VSTParameters(ParameterOwner *iParameterOwner) :
    fParameterOwner{iParameterOwner}
  {
    DCHECK_NOTNULL_F(iParameterOwner);
  };

  // disabling copy
  VSTParameters(VSTParameters const&) = delete;
  VSTParameters& operator=(VSTParameters const &) = delete;

  // Destructor
  ~VSTParameters() = default;

  /**
   * @return the raw parameter given its id
   */
  std::unique_ptr<RawParameter> getRawParameter(ParamID iParamID) const
  {
    return std::make_unique<RawParameter>(iParamID, fParameterOwner);
  }

private:
  ParameterOwner *const fParameterOwner;
};

///////////////////////////////////////////
// Common types
///////////////////////////////////////////


using BooleanParameter = VSTParameter<bool, Common::BooleanParamConverter::denormalize, Common::BooleanParamConverter::normalize>;
using PercentParameter = RawParameter;
template<int StepCount>
using DiscreteParameter = VSTParameter<int, Common::DiscreteValueParamConverter<StepCount>::denormalize, Common::DiscreteValueParamConverter<StepCount>::normalize>;


}
}
}