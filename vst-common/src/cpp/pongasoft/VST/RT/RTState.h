#pragma once

#include "RTParameter.h"

namespace pongasoft {
namespace VST {
namespace RT {

class RTState
{
public:
  template<typename ParamConverter>
  RTParamSPtr<ParamConverter> add(ParamDefSPtr<ParamConverter> iParamDef);
};

template<typename ParamConverter>
RTParamSPtr<ParamConverter> RTState::add(ParamDefSPtr<ParamConverter> iParamDef)
{
  auto rtParam = std::make_shared<RTParameter<ParamConverter>>(iParamDef);
  return rtParam;
}


//template<typename ParamConverter>
//std::shared_ptr<Parameter<ParamConverter>> Plugin::add(Plugin::ParamBuilder<ParamConverter> const &iBuilder)
//{
//auto param = std::make_shared<Parameter<ParamConverter>>(iBuilder.fParamID,
//iBuilder.fTitle,
//iBuilder.fUnits,
//iBuilder.fDefaultNormalizedValue,
//iBuilder.fStepCount,
//iBuilder.fFlags,
//iBuilder.fUnitID,
//iBuilder.fShortTitle,
//iBuilder.fPrecision,
//iBuilder.fUIOnly,
//iBuilder.fSave);
//
//std::shared_ptr<RawParameter> raw = param;
//addRawParameter(raw);
//
//return param;
//}

}
}
}