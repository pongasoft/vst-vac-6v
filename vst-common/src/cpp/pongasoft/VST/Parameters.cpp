#include "Parameters.h"

namespace pongasoft {
namespace VST {

//------------------------------------------------------------------------
// Parameters::build<BooleanParamConverter> => make sure stepCount is 1
//------------------------------------------------------------------------
template<>
Parameters::ParamDefBuilder<BooleanParamConverter> Parameters::build(ParamID iParamID, const TChar *iTitle)
{
  DLOG_F(INFO, "using template special for %d", iParamID);
  return Parameters::ParamDefBuilder<BooleanParamConverter>(this, iParamID, iTitle).stepCount(1);
}


}
}