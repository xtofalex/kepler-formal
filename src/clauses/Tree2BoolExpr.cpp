#include "Tree2BoolExpr.h"
#include "SNLTruthTableTree.h"
#include "SNLTruthTable.h"
#include "BoolExpr.h"

#include <unordered_map>
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <stack>
#include <bitset>    // now required

using namespace naja::NL;
using namespace KEPLER_FORMAL;

// Fold a list of literals into a single AND
static std::shared_ptr<BoolExpr>
mkAnd(const std::vector<std::shared_ptr<BoolExpr>>& lits)
{
    if (lits.empty())
        return BoolExpr::createTrue();
    auto cur = lits[0];
    for (size_t i = 1; i < lits.size(); ++i)
        cur = BoolExpr::And(cur, lits[i]);
    return cur;
}

// Fold a list of terms into a single OR
static std::shared_ptr<BoolExpr>
mkOr(const std::vector<std::shared_ptr<BoolExpr>>& terms)
{
    if (terms.empty())
        return BoolExpr::createFalse();
    auto cur = terms[0];
    for (size_t i = 1; i < terms.size(); ++i)
        cur = BoolExpr::Or(cur, terms[i]);
    return cur;
}

std::shared_ptr<BoolExpr>
KEPLER_FORMAL::Tree2BoolExpr::convert(
    const SNLTruthTableTree&        tree,
    const std::vector<std::string>& varNames)
{
    if (varNames.size() != tree.size())
        throw std::invalid_argument{
            "Tree2BoolExpr: varNames.size() != tree.size()"
        };

    using NodeKey = const ITTNode*;
    std::unordered_map<NodeKey,std::shared_ptr<BoolExpr>> memo;
    NodeKey root = tree.getRoot();

    // stack of (node, visited–flag)
    std::vector<std::pair<NodeKey,bool>> stack;
    stack.reserve(1024);
    stack.emplace_back(root, false);

    while (!stack.empty()) {
        auto [node, visited] = stack.back();
        stack.pop_back();

        // if already computed, skip
        if (memo.count(node)) 
            continue;

        // First time we see this node?
        if (!visited) {
            // For table nodes, push post–visit marker then children
            if (auto tn = dynamic_cast<const TableNode*>(node)) {
                stack.emplace_back(node, true);
                for (auto& child : tn->children)
                    stack.emplace_back(child.get(), false);
            }
            // Input nodes we can handle immediately
            else if (dynamic_cast<const InputNode*>(node)) {
                auto in = static_cast<const InputNode*>(node);
                memo[node] = BoolExpr::Var(varNames[in->inputIndex]);
            }
            else {
                throw std::logic_error{
                    "Tree2BoolExpr: unknown ITTNode subtype"
                };
            }
        }
        // Post–visit: all children are in memo, now build this node
        else {
            // must be a table node here
            auto tn = dynamic_cast<const TableNode*>(node);
            const SNLTruthTable& tbl = tn->table;
            uint32_t k    = tbl.size();
            uint64_t rows = uint64_t{1} << k;

            // constant shortcuts
            if (tbl.all0()) {
                memo[node] = BoolExpr::createFalse();
            }
            else if (tbl.all1()) {
                memo[node] = BoolExpr::createTrue();
            }
            else {
                // gather child expressions
                std::vector<std::shared_ptr<BoolExpr>> childF(k);
                for (uint32_t i = 0; i < k; ++i)
                    childF[i] =
                        memo.at(tn->children[i].get());

                // build DNF: one term per “true” row
                std::vector<std::shared_ptr<BoolExpr>> terms;
                terms.reserve(rows);
                for (uint64_t m = 0; m < rows; ++m) {
                    if (!tbl.bits().bit(m))
                        continue;
                    // build conjunction for this row
                    std::vector<std::shared_ptr<BoolExpr>> lits;
                    lits.reserve(k);
                    for (uint32_t j = 0; j < k; ++j) {
                        bool bitIs1 = ((m >> j) & 1) != 0;
                        lits.push_back(
                          bitIs1
                            ? childF[j]
                            : BoolExpr::Not(childF[j])
                        );
                    }
                    terms.push_back(mkAnd(lits));
                }
                memo[node] = mkOr(terms);
            }
        }
    }

    // root expression now in memo
    return memo.at(root);
}
