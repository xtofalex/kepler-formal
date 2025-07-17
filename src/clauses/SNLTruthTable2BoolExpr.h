#include "BoolExpr.h"
#include "SNLTruthTable.h"
// for shared_ptr
#include <memory>

namespace KEPLER_FORMAL {

std::shared_ptr<KEPLER_FORMAL::BoolExpr> TruthTableToBoolExpr(
    const naja::NL::SNLTruthTable &tt,
    const std::vector<std::string> &varNames);

}