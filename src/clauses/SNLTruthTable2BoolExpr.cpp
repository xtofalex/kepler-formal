#include "SNLTruthTable.h"
#include "BoolExpr.h"
#include <memory>
#include <string>

// Assumes your BoolExpr factory methods live in namespace KEPLER_FORMAL
using namespace KEPLER_FORMAL;

// Convert an SNLTruthTable → BoolExpr (sum-of-products)
std::shared_ptr<BoolExpr> TruthTableToBoolExpr(
    const naja::NL::SNLTruthTable &tt,
    const std::vector<std::string> &varNames)
{
    uint32_t n = tt.size();
    uint32_t rows = 1u << n;

    // Edge cases: no variables
    if (n == 0) {
        // table has exactly one output bit
        bool out0 = tt.bits().bit(0);
        // we don’t have explicit constants, so use a dummy var that’s always out0.
        auto v = BoolExpr::Var(out0 ? "TRUE" : "FALSE");
        return out0 ? v : BoolExpr::Not(v);
    }

    // 1) Collect all cubes for minterms == 1
    std::vector<std::shared_ptr<BoolExpr>> cubes;
    for (uint32_t row = 0; row < rows; ++row) {
        if (!tt.bits().bit(row)) 
            continue;

        // Build conjunction for this minterm
        std::shared_ptr<BoolExpr> cube;
        for (uint32_t bit = 0; bit < n; ++bit) {
            // pick the appropriate literal
            bool litVal = (row >> bit) & 1u;
            auto varExpr = BoolExpr::Var(varNames.at(bit));
            auto litExpr = litVal ? varExpr
                                  : BoolExpr::Not(varExpr);

            cube = cube
                 ? BoolExpr::And(cube, litExpr)
                 : litExpr;
        }
        cubes.push_back(cube);
    }

    // 2) If no minterms are true → constant 0
    if (cubes.empty()) {
        auto v = BoolExpr::Var("FALSE");
        return BoolExpr::Not(v);
    }

    // 3) OR together all the cubes
    auto expr = cubes.front();
    for (size_t i = 1; i < cubes.size(); ++i) {
        expr = BoolExpr::Or(expr, cubes[i]);
    }
    return expr;
}
