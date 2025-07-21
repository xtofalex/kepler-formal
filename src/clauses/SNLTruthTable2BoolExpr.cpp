#include "SNLTruthTable.h"
#include "SNLTruthTable2BoolExpr.h"
#include "BoolExpr.h"
#include <memory>
#include <string>

namespace KEPLER_FORMAL {

std::shared_ptr<BoolExpr> TruthTableToBoolExpr::convert(
    const naja::NL::SNLTruthTable& tt,
    const std::vector<std::string>& varNames)
{
    uint32_t n = tt.size();
    uint32_t rows = 1u << n;

    if (varNames.size() < n) {
        printf("VarNames vector size (%zu) is less than truth table size (%u).\n",
               varNames.size(), n);
        throw std::invalid_argument("varNames vector size must match truth table size.");
    }

    // Handle zero-variable truth tables
    if (n == 0) {
        bool out0 = tt.bits().bit(0);
        auto v = BoolExpr::Var(out0 ? "TRUE" : "FALSE");
        return out0 ? v : BoolExpr::Not(v);
    }

    std::vector<std::shared_ptr<BoolExpr>> cubes;

    for (uint32_t row = 0; row < rows; ++row) {
        if (!tt.bits().bit(row)) continue;

        std::shared_ptr<BoolExpr> cube;

        for (uint32_t bit = 0; bit < n; ++bit) {
            bool val = (row >> bit) & 1;
            auto var = BoolExpr::Var(varNames.at(bit));
            auto lit = val ? var : BoolExpr::Not(var);
            cube = cube ? BoolExpr::And(cube, lit) : lit;
        }

        cubes.push_back(cube);
    }

    if (cubes.empty()) {
        auto v = BoolExpr::Var("FALSE");
        return BoolExpr::Not(v);
    }

    auto expr = cubes.front();
    for (size_t i = 1; i < cubes.size(); ++i) {
        expr = BoolExpr::Or(expr, cubes[i]);
    }

    return expr;
}

}
