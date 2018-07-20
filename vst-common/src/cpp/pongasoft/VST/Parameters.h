#pragma once

#include <map>
#include <vector>
#include "ParamDef.h"

namespace pongasoft {
namespace VST {

/**
 * This is the class which maintains all the registered parameters
 */
class Parameters
{
public:
  template<typename ParamConverter>
  struct ParamDefBuilder
  {
    // builder methods
    ParamDefBuilder &units(const TChar *iUnits) { fUnits = iUnits; return *this; }
    ParamDefBuilder &defaultValue(typename ParamConverter::ParamType const &iDefaultValue) { fDefaultNormalizedValue = ParamConverter::normalize(iDefaultValue); return *this;}
    ParamDefBuilder &stepCount(int32 iStepCount) { fStepCount = iStepCount; return *this; }
    ParamDefBuilder &flags(int32 iFlags) { fFlags = iFlags; return *this; }
    ParamDefBuilder &unitID(int32 iUnitID) { fUnitID = iUnitID; return *this; }
    ParamDefBuilder &shortTitle(const TChar *iShortTitle) { fShortTitle = iShortTitle; return *this; }
    ParamDefBuilder &precision(int32 iPrecision) { fPrecision = iPrecision; return *this; }
    ParamDefBuilder &uiOnly(bool iUIOnly) { fUIOnly = iUIOnly; return *this; }
    ParamDefBuilder &save(bool iSave) { fSave = iSave; return *this; }

    // parameter factory method
    ParamDefSPtr<ParamConverter> add() const;

    // fields
    ParamID fParamID;
    const TChar *fTitle;
    const TChar *fUnits = nullptr;
    ParamValue fDefaultNormalizedValue = 0;
    int32 fStepCount = 0;
    int32 fFlags = ParameterInfo::kCanAutomate;
    UnitID fUnitID = kRootUnitId;
    const TChar *fShortTitle = nullptr;
    int32 fPrecision = 4;
    bool fUIOnly = false;
    bool fSave = true;

    friend class Parameters;

  protected:
    ParamDefBuilder(Parameters *iParameters, ParamID iParamID, const TChar* iTitle) : fParameters{iParameters}, fParamID{iParamID}, fTitle{iTitle} {}

  private:
    Parameters *fParameters;
  };

public:
  // Constructor
  explicit Parameters() {};

  /**
   * Used from derived classes to build a parameter.
   * TODO add example + don't forget that order is important!
   */
  template<typename ParamConverter>
  ParamDefBuilder<ParamConverter> build(ParamID iParamID, const TChar* iTitle);

protected:
  // internally called by the builder
  template<typename ParamConverter>
  ParamDefSPtr<ParamConverter> add(ParamDefBuilder<ParamConverter> const &iBuilder);

  // add raw parameter to the structures
  void addRawParamDef(std::shared_ptr<RawParamDef> iParameter);

private:
  // contains all the registered parameters (unique ID, will be checked on add)
  std::map<ParamID, std::shared_ptr<RawParamDef>> fParameters{};
  // maintains the insertion order and keeps track of parameters saved by the RT
  std::vector<std::shared_ptr<RawParamDef>> fRTParameters{};
  // maintains the insertion order and keeps track of parameters saved by the GUI (and not used by the RT)
  std::vector<std::shared_ptr<RawParamDef>> fGUIParameters{};
};

//------------------------------------------------------------------------
// Parameters::ParamDefBuilder::add
//------------------------------------------------------------------------
template<typename ParamConverter>
ParamDefSPtr<ParamConverter> Parameters::ParamDefBuilder<ParamConverter>::add() const
{
  return fParameters->add(*this);
}

//------------------------------------------------------------------------
// Parameters::add (called by the builder)
//------------------------------------------------------------------------
template<typename ParamConverter>
ParamDefSPtr<ParamConverter> Parameters::add(ParamDefBuilder<ParamConverter> const &iBuilder)
{
  auto param = std::make_shared<ParamDef<ParamConverter>>(iBuilder.fParamID,
                                                          iBuilder.fTitle,
                                                          iBuilder.fUnits,
                                                          iBuilder.fDefaultNormalizedValue,
                                                          iBuilder.fStepCount,
                                                          iBuilder.fFlags,
                                                          iBuilder.fUnitID,
                                                          iBuilder.fShortTitle,
                                                          iBuilder.fPrecision,
                                                          iBuilder.fUIOnly,
                                                          iBuilder.fSave);

  std::shared_ptr<RawParamDef> raw = param;

//  addRawParamDef(raw);

  return param;
}

//------------------------------------------------------------------------
// Parameters::build
//------------------------------------------------------------------------
template<typename ParamConverter>
Parameters::ParamDefBuilder<ParamConverter> Parameters::build(ParamID iParamID, const TChar *iTitle)
{
  return Parameters::ParamDefBuilder<ParamConverter>(this, iParamID, iTitle);
}


//------------------------------------------------------------------------
// Parameters::addRawParameter TODO (add?)
//------------------------------------------------------------------------
//void Parameters::addRawParameter(std::shared_ptr<RawParamDef> iParameter)
//{
//  DCHECK_F(iParameter != nullptr);
//  DCHECK_F(fParameters.find(iParameter->fParamID) == fParameters.cend());
//  fParameters[iParameter->fParamID] = iParameter;
//  if(iParameter->fUIOnly)
//    fGUIParameters.emplace_back(iParameter);
//  else
//    fRTParameters.emplace_back(iParameter);
//}

}
}