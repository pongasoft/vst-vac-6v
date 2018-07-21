#ifndef __PONGASOFT_VST_PARAM_DEF_H__
#define __PONGASOFT_VST_PARAM_DEF_H__

#include "ParamConverters.h"

#include <base/source/fstreamer.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <pluginterfaces/vst/ivstunits.h>

#include <memory>

namespace pongasoft {
namespace VST {

using namespace Steinberg;
using namespace Steinberg::Vst;

class RawParamDef
{
public:
  RawParamDef(ParamID const iParamID,
              TChar const *const iTitle,
              TChar const *const iUnits,
              ParamValue const iDefaultNormalizedValue,
              int32 const iStepCount,
              int32 const iFlags,
              UnitID const iUnitID,
              TChar const *const iShortTitle,
              int32 const iPrecision,
              bool const iUIOnly,
              bool const iTransient) :
    fParamID{iParamID},
    fTitle{iTitle},
    fUnits{iUnits},
    fDefaultNormalizedValue{iDefaultNormalizedValue},
    fStepCount{iStepCount},
    fFlags{iFlags},
    fUnitID{iUnitID},
    fShortTitle{iShortTitle},
    fPrecision{iPrecision},
    fUIOnly{iUIOnly},
    fTransient{iTransient}
  {}

public:
  ParamValue readNormalizedValue(IBStreamer &iStreamer) const
  {
    if(!fTransient)
    {
      double value;
      if(!iStreamer.readDouble(value))
        value = fDefaultNormalizedValue;
      return value;
    }
    else
      return fDefaultNormalizedValue;
  }

public:
  const ParamID fParamID;
  const TChar *const fTitle;
  const TChar *const fUnits;
  const ParamValue fDefaultNormalizedValue;
  const int32 fStepCount;
  const int32 fFlags;
  const UnitID fUnitID;
  const TChar *const fShortTitle;
  const int32 fPrecision;
  const bool fUIOnly;
  const bool fTransient;
};

template<typename ParamConverter>
class ParamDef : public RawParamDef
{
public:
  using ParamType = typename ParamConverter::ParamType;

  ParamDef(ParamID const iParamID,
           TChar const *const iTitle,
           TChar const *const iUnits,
           ParamValue const iDefaultNormalizedValue,
           int32 const iStepCount,
           int32 const iFlags,
           UnitID const iUnitID,
           TChar const *const iShortTitle,
           int32 const iPrecision,
           bool const iUIOnly,
           bool const iTransient) :
    RawParamDef(iParamID,
                iTitle,
                iUnits,
                iDefaultNormalizedValue,
                iStepCount,
                iFlags,
                iUnitID,
                iShortTitle,
                iPrecision,
                iUIOnly,
                iTransient)
  {
  }

  inline ParamType getDefaultValue() const { return denormalize(fDefaultNormalizedValue); }
  inline ParamValue normalize(ParamType const &iValue) const { return ParamConverter::normalize(iValue); }
  inline ParamType denormalize(ParamValue iNormalizedValue) const { return ParamConverter::denormalize(iNormalizedValue); }

};

template<typename ParamConverter>
using ParamDefSPtr = std::shared_ptr<ParamDef<ParamConverter>>;

//template<typename ParamConverter, typename S>
//class Binder
//{
//public:
//  using ParamType = typename ParamConverter::ParamType;
//
//  Binder(ParamType S::*iBoundPtr) : fBoundPtr{iBoundPtr} {}
//
//  bool setState(ParamValue iNormalizedValue, S &oState)
//  {
//    ParamValue previousNormalizedValue = ParamConverter::normalize(oState.*fBoundPtr);
//    if(previousNormalizedValue != iNormalizedValue)
//    {
//      oState.*fBoundPtr = ParamConverter::denormalize(iNormalizedValue);
//      return true;
//    }
//
//    return false;
//  }
//
//private:
//  ParamType S::*fBoundPtr;
//};

}
}

#endif // __PONGASOFT_VST_PARAM_DEF_H__