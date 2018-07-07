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
    Editor(ParamID iParamID, ParameterOwner *iParameterOwner);

    // disabling copy
    Editor(Editor const &) = delete;
    Editor& operator=(Editor const &) = delete;

    /**
     * Change the value of the parameter. Note that nothing happens if you have called commit or rollback already
     */
    tresult setValue(ParamValue iValue);

    /*
     * Call when you are done with the modifications.
     * This has no effect if rollback() has already been called
     */
    tresult commit();

    /*
     * Shortcut to set the value prior to commit
     * Call when you are done with the modifications.
     * This has no effect if rollback() has already been called
     */
    inline tresult commit(ParamValue iValue)
    {
      setValue(iValue);
      return commit();
    }

    /**
     * Call this if you want to revert to the original value of the parameter (when the editor is created).
     * This has no effect if commit() has already been called
     */
    tresult rollback();

    /**
     * Destructor which calls rollback by default
     */
    inline ~Editor()
    {
      // DLOG_F(INFO, "~RawParameter::Editor(%d)", fParamID);
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
    Connection(ParamID iParamID, ParameterOwner *iParameterOwner, IChangeListener *iChangeListener);

    /**
     * Call to stop listening for changes. Also called automatically from the destructor.
     */
    void close();

    /**
     * Automatically closes the connection and stops listening */
    inline ~Connection() override
    {
      // DLOG_F(INFO, "~RawParameter::Connection(%d)", fParamID);
      close();
    }

    /**
     * This is being called when the parameter receives a message... do not call explicitely
     */
    void PLUGIN_API update(FUnknown *iChangedUnknown, Steinberg::int32 iMessage) SMTG_OVERRIDE;

    // disabling copy
    Connection(Connection const &) = delete;
    Connection& operator=(Connection const &) = delete;

  private:
    ParamID fParamID;
    Parameter *fParameter;
    ParameterOwner *const fParameterOwner;
    IChangeListener *const fChangeListener;
    bool fIsConnected;
  };

public:
  // Constructor
  RawParameter(ParamID iParamID, ParameterOwner *iParameterOwner);

  // Destructor
  ~RawParameter()
  {
    // DLOG_F(INFO, "RawParameter::~RawParameter(%d)", fParamID);
  }

  // getParamID
  inline ParamID getParamID() const
  {
    return fParamID;
  }

  /**
   * @return the current raw value of the parameter
   */
  inline ParamValue getValue() const
  {
    return fParameterOwner->getParamNormalized(fParamID);
  }

  /**
   * Sets the value of this parameter. Note that this is "transactional" and if you want to make
   * further changes that spans multiple calls (ex: onMouseDown / onMouseMoved / onMouseUp) you should use an editor
   */
  inline tresult setValue(ParamValue iValue)
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
   * Shortcut to create an editor and set the value to it
   *
   * @return an editor to modify the parameter (see Editor)
   */
  std::unique_ptr<Editor> edit(ParamValue iValue)
  {
    auto editor = edit();
    editor->setValue(iValue);
    return editor;
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
// VSTParameter<T>
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
    // DLOG_F(INFO, "VSTParameter::VSTParameter(%d)", fRawParameter->getParamID());
  }

  // Destructor
  ~VSTParameter()
  {
    // DLOG_F(INFO, "VSTParameter::~VSTParameter(%d)", fRawParameter->getParamID());
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
    auto editor = edit();
    editor->setValue(iValue);
    return editor;
  }

private:
  std::unique_ptr<RawParameter> fRawParameter;
};

///////////////////////////////////////////
// class VSTParameters
///////////////////////////////////////////
class VSTParametersManager;

/**
 * Encapsulates access to the vst parameters defined in the controller
 */
class VSTParameters : public std::enable_shared_from_this<VSTParameters>
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

  /**
   * @return the raw parameter given its id
   */
  std::unique_ptr<RawParameter> getRawParameter(ParamID iParamID) const
  {
    return std::make_unique<RawParameter>(iParamID, fParameterOwner);
  }

  /**
   * Creates a manager
   */
  std::unique_ptr<VSTParametersManager> createManager();

private:
  ParameterOwner *const fParameterOwner;
};

///////////////////////////////////////////
// Common types
///////////////////////////////////////////

template<typename T, typename U>
using VSTParameterFromType = VSTParameter<T, U::denormalize, U::normalize>;

template<typename T>
using VSTParameterFromClass = VSTParameterFromType<T, T>;

using BooleanParameter = VSTParameter<bool, Common::BooleanParamConverter::denormalize, Common::BooleanParamConverter::normalize>;
using PercentParameter = RawParameter;
template<int StepCount>
using DiscreteParameter = VSTParameter<int, Common::DiscreteValueParamConverter<StepCount>::denormalize, Common::DiscreteValueParamConverter<StepCount>::normalize>;

///////////////////////////////////////////
// class VSTParametersManager
///////////////////////////////////////////

class VSTParametersManager
{
public:
  /**
   * Registers a raw parameter (no conversion)
   */
  std::unique_ptr<RawParameter> registerRawParameter(ParamID iParamID,
                                                     RawParameter::IChangeListener *iChangeListener = nullptr);

  /**
   * Generic register with any kind of conversion
   */
  template<typename T>
  std::unique_ptr<T> registerVSTParameter(ParamID iParamID,
                                          RawParameter::IChangeListener *iChangeListener = nullptr)
  {
    return std::make_unique<T>(registerRawParameter(iParamID, iChangeListener));
  }

  // shortcut for BooleanParameter
  std::unique_ptr<BooleanParameter> registerBooleanParameter(ParamID iParamID,
                                                             RawParameter::IChangeListener *iChangeListener = nullptr);

  // shortcut for PercentParameter
  std::unique_ptr<PercentParameter> registerPercentParameter(ParamID iParamID,
                                                             RawParameter::IChangeListener *iChangeListener = nullptr);

  friend class VSTParameters;

protected:
  explicit VSTParametersManager(std::shared_ptr<VSTParameters> iParameters) :
    fParameters{std::move(iParameters)}
  {}

private:

  std::shared_ptr<VSTParameters> fParameters;

  // Maintains the connections for the listeners... will be automatically discarded in the destructor
  std::map<ParamID, std::unique_ptr<RawParameter::Connection>> fParameterConnections;
};


}
}
}