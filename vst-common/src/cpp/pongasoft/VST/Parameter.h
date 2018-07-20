#pragma once

#include <base/source/fstreamer.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <pluginterfaces/vst/ivstunits.h>
#include <memory>
#include "ParamConverters.h"

namespace pongasoft {
namespace VST {

using namespace Steinberg;
using namespace Steinberg::Vst;

class RawParameter
{
public:
  RawParameter(ParamID const iParamID,
               TChar const *const iTitle,
               TChar const *const iUnits,
               ParamValue const iDefaultNormalizedValue,
               int32 const iStepCount,
               int32 const iFlags,
               UnitID const iUnitID,
               TChar const *const iShortTitle,
               int32 const iPrecision,
               bool const iUIOnly,
               bool const iSave) :
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
    fSave{iSave}
  {}

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
  const bool fSave;
};

template<typename ParamConverter>
class Parameter : public RawParameter
{
public:
  using ParamType = typename ParamConverter::ParamType;

  Parameter(ParamID const iParamID,
            TChar const *const iTitle,
            TChar const *const iUnits,
            ParamValue const iDefaultNormalizedValue,
            int32 const iStepCount,
            int32 const iFlags,
            UnitID const iUnitID,
            TChar const *const iShortTitle,
            int32 const iPrecision,
            bool const iUIOnly,
            bool const iSave) :
    RawParameter(iParamID,
                 iTitle,
                 iUnits,
                 iDefaultNormalizedValue,
                 iStepCount,
                 iFlags,
                 iUnitID,
                 iShortTitle,
                 iPrecision,
                 iUIOnly,
                 iSave)
  {
  }

  inline ParamValue normalize(ParamType const &iValue) const
  {
    return ParamConverter::normalize(iValue);
  }

  inline ParamType denormalize(ParamValue iNormalizedValue) const
  {
    return ParamConverter::denormalize(iNormalizedValue);
  }

  ParamValue readNormalizedValue(IBStreamer &iStreamer) const;

  bool rtSetState(ParamValue iNormalizedValue, ParamType &oState);
  bool rtReadState(IBStreamer &iStreamer, ParamType &oState);
  void rtWriteState(ParamType const &iState, IBStreamer &oStreamer);
};

template<typename ParamConverter>
ParamValue Parameter<ParamConverter>::readNormalizedValue(IBStreamer &iStreamer) const
{
  double value;
  if(!iStreamer.readDouble(value))
    value = normalize(fDefaultNormalizedValue);
  return value;
}

template<typename ParamConverter>
bool Parameter<ParamConverter>::rtSetState(ParamValue iNormalizedValue, ParamType &oState)
{
  ParamValue previousNormalizedValue = normalize(oState);
  if(previousNormalizedValue != iNormalizedValue)
  {
    oState = denormalize(iNormalizedValue);
    return true;
  }

  return false;
}

template<typename ParamConverter>
bool Parameter<ParamConverter>::rtReadState(IBStreamer &iStreamer, ParamType &oState)
{
  return rtSetState(readNormalizedValue(iStreamer), oState);
}

template<typename ParamConverter>
void Parameter<ParamConverter>::rtWriteState(ParamType const &iState, IBStreamer &oStreamer)
{
  oStreamer.writeDouble(normalize(iState));
}

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