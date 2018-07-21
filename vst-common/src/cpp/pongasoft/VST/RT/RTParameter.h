#include <pluginterfaces/vst/ivstaudioprocessor.h>

#include <utility>

#ifndef __PONGASOFT_VST_RT_PARAMETER_H__
#define __PONGASOFT_VST_RT_PARAMETER_H__

#include <pongasoft/VST/ParamDef.h>
#include <pongasoft/logging/loguru.hpp>

namespace pongasoft {
namespace VST {
namespace RT {

/**
 * RTRawParameter
 */
class RTRawParameter
{
public:
  explicit RTRawParameter(std::shared_ptr<RawParamDef> iParamDef) :
    fRawParamDef{iParamDef},
    fNormalizedValue{fRawParamDef->fDefaultNormalizedValue},
    fPreviousNormalizedValue{fNormalizedValue}
  {}

  ParamID getParamID() const { return fRawParamDef->fParamID; }
  std::shared_ptr<RawParamDef> getRawParamDef() const { return fRawParamDef; }

  virtual bool updateNormalizedValue(ParamValue iNormalizedValue);

  inline ParamValue const &getNormalizedValue() { return fNormalizedValue; }
  inline ParamValue const &getPreviousNormalizedValue() { return fPreviousNormalizedValue; }

  tresult addToOutput(ProcessData &oData);

  inline bool hasChanged() const { return fNormalizedValue != fPreviousNormalizedValue; }

  virtual bool resetPreviousValue();

protected:
  std::shared_ptr<RawParamDef> fRawParamDef;
  ParamValue fNormalizedValue;
  ParamValue fPreviousNormalizedValue;
};


/**
 * RTParameter
 */
template<typename ParamConverter>
class RTParameter : public RTRawParameter
{
public:
  using ParamType = typename ParamConverter::ParamType;

  explicit RTParameter(ParamDefSPtr<ParamConverter> iParamDef) :
    RTRawParameter(iParamDef),
    fValue{denormalize(fNormalizedValue)},
    fPreviousValue{fValue}
  {
    DLOG_F(INFO, "RTParameter(%s)", String(iParamDef->fTitle).text8());
  }

  bool updateNormalizedValue(ParamValue iNormalizedValue) override;

  inline ParamValue normalize(ParamType const &iValue) const { return ParamConverter::normalize(iValue); }
  inline ParamType denormalize(ParamValue iNormalizedValue) const { return ParamConverter::denormalize(iNormalizedValue); }

  void update(ParamType const &iNewValue);

  inline ParamType const &v() const { return fValue; }
  inline ParamType const &pv() const { return fPreviousValue; }

protected:
  bool resetPreviousValue() override;

protected:
  ParamType fValue;
  ParamType fPreviousValue;
};

//------------------------------------------------------------------------
// RTParameter::updateNormalizedValue - update fValue to the new value and return true if it changed
//------------------------------------------------------------------------
template<typename ParamConverter>
bool RTParameter<ParamConverter>::updateNormalizedValue(ParamValue iNormalizedValue)
{
  if(RTRawParameter::updateNormalizedValue(iNormalizedValue))
  {
    fValue = denormalize(iNormalizedValue);
    return true;
  }

  return false;
}

//------------------------------------------------------------------------
// RTParameter::resetPreviousValue
//------------------------------------------------------------------------
template<typename ParamConverter>
bool RTParameter<ParamConverter>::resetPreviousValue()
{
  if(RTRawParameter::resetPreviousValue())
  {
    fPreviousValue = fValue;
    return true;
  }

  return false;
}

//------------------------------------------------------------------------
// RTParameter::update
//------------------------------------------------------------------------
template<typename ParamConverter>
void RTParameter<ParamConverter>::update(const ParamType &iNewValue)
{
  fValue = iNewValue;
  fNormalizedValue = normalize(fValue);
}

//------------------------------------------------------------------------
// RTParamSPtr - wrapper to make writing the code much simpler and natural
//------------------------------------------------------------------------
template<typename ParamConverter>
class RTParamSPtr
{
  using ParamType = typename ParamConverter::ParamType;

public:
  RTParamSPtr(std::shared_ptr<RTParameter<ParamConverter>> iPtr) :
    fPtr{std::move(iPtr)}
  {}

  inline void update(ParamType const &iNewValue) { fPtr->update(iNewValue); }
  inline bool hasChanged() const { return fPtr->hasChanged(); }
  inline tresult addToOutput(ProcessData &oData) { return fPtr->addToOutput(oData); }

  // allow to use the param as the underlying ParamType (ex: "if(param)" in the case ParamType is bool))
  inline operator ParamType const &() const { return fPtr->v(); }
  // allow writing param->xxx to access the underlying type directly (if not a primitive)
  inline ParamType const *operator->() const { return &fPtr->v(); }

private:
  std::shared_ptr<RTParameter<ParamConverter>> fPtr;
};

}
}
}

#endif // __PONGASOFT_VST_RT_PARAMETER_H__