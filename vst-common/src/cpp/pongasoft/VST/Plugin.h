#pragma once

#include <map>
#include <vector>
#include "ParamDef.h"

namespace pongasoft {
namespace VST {

/**
 * Main Plugin class, where the parameters are registered and managed
 */
class Plugin
{
};

//public:
//enum class Type
//{
//  kRT,
//  kGUI
//};
//
//template<typename ParamConverter>
//struct ParamBuilder
//{
//  // builder methods
//  ParamBuilder &units(const TChar *iUnits) { fUnits = iUnits; return *this; }
//  ParamBuilder &defaultValue(typename ParamConverter::ParamType const &iDefaultValue) { fDefaultNormalizedValue = ParamConverter::normalize(iDefaultValue); return *this;}
//  ParamBuilder &stepCount(int32 iStepCount) { fStepCount = iStepCount; return *this; }
//  ParamBuilder &flags(int32 iFlags) { fFlags = iFlags; return *this; }
//  ParamBuilder &unitID(int32 iUnitID) { fUnitID = iUnitID; return *this; }
//  ParamBuilder &shortTitle(const TChar *iShortTitle) { fShortTitle = iShortTitle; return *this; }
//  ParamBuilder &precision(int32 iPrecision) { fPrecision = iPrecision; return *this; }
//  ParamBuilder &uiOnly(bool iUIOnly) { fUIOnly = iUIOnly; return *this; }
//  ParamBuilder &save(bool iSave) { fSave = iSave; return *this; }
//
//  // parameter factory method
//  std::shared_ptr<Parameter<ParamConverter>> add() const;
//
//  // fields
//  ParamID fParamID;
//  const TChar *fTitle;
//  const TChar *fUnits = nullptr;
//  ParamValue fDefaultNormalizedValue = 0;
//  int32 fStepCount = 0;
//  int32 fFlags = ParameterInfo::kCanAutomate;
//  UnitID fUnitID = kRootUnitId;
//  const TChar *fShortTitle = nullptr;
//  int32 fPrecision = 4;
//  bool fUIOnly = false;
//  bool fSave = true;
//
//  friend class Plugin;
//
//protected:
//  ParamBuilder(Plugin *iPlugin, ParamID iParamID, const TChar* iTitle) : fPlugin{iPlugin}, fParamID{iParamID}, fTitle{iTitle} {}
//
//private:
//  Plugin *fPlugin;
//};
//
//public:
//// Constructor
//explicit Plugin(Type iType) : fType{iType} {};
//
///**
// * Used from derived classes to build a parameter.
// * TODO add example + don't forget that order is important!
// */
//template<typename ParamConverter>
//ParamBuilder<ParamConverter> build(ParamID iParamID, const TChar* iTitle);
//
//protected:
//// internally called by the builder
//template<typename ParamConverter>
//std::shared_ptr<Parameter<ParamConverter>> add(ParamBuilder<ParamConverter> const &iBuilder);
//
//// add raw parameter to the structures
//void addRawParameter(std::shared_ptr<RawParameter> iParameter);
//
//protected:
//Type const fType;
//
//private:
//// contains all the registered parameters (unique ID, will be checked on add)
//std::map<ParamID, std::shared_ptr<RawParameter>> fParameters{};
//// maintains the insertion order and keeps track of parameters saved by the RT
//std::vector<std::shared_ptr<RawParameter>> fRTParameters{};
//// maintains the insertion order and keeps track of parameters saved by the GUI (and not used by the RT)
//std::vector<std::shared_ptr<RawParameter>> fGUIParameters{};
//};
//
////------------------------------------------------------------------------
//// Plugin::ParamBuilder::add
////------------------------------------------------------------------------
//template<typename ParamConverter>
//std::shared_ptr<Parameter<ParamConverter>> Plugin::ParamBuilder<ParamConverter>::add() const
//{
//  return fPlugin->add(*this);
//}
//
////------------------------------------------------------------------------
//// Plugin::add (called by the builder)
////------------------------------------------------------------------------
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
//
////------------------------------------------------------------------------
//// Plugin::build
////------------------------------------------------------------------------
//template<typename ParamConverter>
//Plugin::ParamBuilder<ParamConverter> Plugin::build(ParamID iParamID, const TChar *iTitle)
//{
//  return Plugin::ParamBuilder<ParamConverter>(this, iParamID, iTitle);
//}


}
}