#pragma once

#include <pongasoft/VST/ParamDef.h>
#include <pongasoft/logging/loguru.hpp>

namespace pongasoft {
namespace VST {
namespace RT {

class RTRawParameter
{
public:
  virtual bool update(ParamValue iNormalizedValue) = 0;
  virtual ParamValue getNormalizedValue() const = 0;
};

template<typename ParamConverter>
class RTParameter : public RTRawParameter
{
public:
  using ParamType = typename ParamConverter::ParamType;

  explicit RTParameter(ParamDefSPtr<ParamConverter> iParam) :
    fParam{iParam},
    fValue{iParam->getDefaultValue()},
    fPreviousValue{fValue}
  {
    DLOG_F(INFO, "RTParameter(%s)", String(fParam->fTitle).text8());
  }

  bool update(ParamValue iNormalizedValue) override;
  ParamValue getNormalizedValue() const override { return normalize(fValue); }

  inline ParamValue normalize(ParamType const &iValue) const { return ParamConverter::normalize(iValue); }
  inline ParamType denormalize(ParamValue iNormalizedValue) const { return ParamConverter::denormalize(iNormalizedValue); }

public:
  ParamType fValue;
  ParamType fPreviousValue;

protected:
  ParamDefSPtr<ParamConverter> fParam;
};

template<typename ParamConverter>
bool RTParameter<ParamConverter>::update(ParamValue iNormalizedValue)
{
  ParamValue previousNormalizedValue = normalize(fValue);
  if(previousNormalizedValue != iNormalizedValue)
  {
    fValue = denormalize(iNormalizedValue);
    return true;
  }

  return false;
}

template<typename ParamConverter>
using RTParamSPtr = std::shared_ptr<RTParameter<ParamConverter>>;

//template<typename ParamConverter>
//bool RTParameter<ParamConverter>::read(IBStreamer &iStreamer)
//{
//  if(fParam->fSave)
//    return update(fParam->readNormalizedValue(iStreamer));
//  else
//    return false;
//}
//
//template<typename ParamConverter>
//void RTParameter<ParamConverter>::write(IBStreamer &oStreamer)
//{
//  if(fParam->fSave)
//    oStreamer.writeDouble(fParam->normalize(fValue));
//
//}

}
}
}