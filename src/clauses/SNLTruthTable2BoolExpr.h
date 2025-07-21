#include "BoolExpr.h"
#include "SNLTruthTable.h"
// for shared_ptr
#include <memory>

namespace KEPLER_FORMAL {

class TruthTableToBoolExpr {

public:

static std::shared_ptr<KEPLER_FORMAL::BoolExpr> convert(
    const naja::NL::SNLTruthTable &tt,
    const std::vector<std::string> &varNames);

};

}