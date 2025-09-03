// Tree2BoolExpr.cpp

#include "Tree2BoolExpr.h"
#include "SNLTruthTableTree.h"     // for SNLTruthTableTree::Node
#include "SNLTruthTable.h"
#include "BoolExpr.h"

#include <unordered_map>
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <utility>   // for std::pair
#include <bitset>    // needed for bit‐manipulation

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

    // We're now indexing into the unified Node
    using NodeKey = const SNLTruthTableTree::Node*;

    std::unordered_map<size_t,std::shared_ptr<BoolExpr>> memo;
    const SNLTruthTableTree::Node* root = tree.getRoot();

    // manual post‐order traversal stack of (node, visited?)
    std::vector<std::pair<NodeKey,bool>> stack;
    stack.reserve(1024);
    stack.emplace_back(root, false);

    while (!stack.empty()) {
        auto [node, visited] = stack.back();
        stack.pop_back();

        // if we already computed it, skip
        if (memo.count(node->nodeID))
            continue;

        // first time we see this node?
        if (!visited) {
            if (node->type == SNLTruthTableTree::Node::Type::Table) {
                // push post‐visit marker
                stack.emplace_back(node, true);
                // push children for pre‐visit
                for (auto& child : node->children)
                    stack.emplace_back(child.get(), false);
            }
            else {
                // Input leaf
                // (no need for a visited pass)
                size_t idx = node->inputIndex;
                memo[node->nodeID] = BoolExpr::Var(varNames[idx]);
            }
        }
        else {
            // second time we see this node: build its TableExpr
            // must be a Table node
            const auto& tbl = node->table;
            uint32_t k      = tbl.size();
            uint64_t rows   = uint64_t{1} << k;

            // constant‐table shortcuts
            if (tbl.all0()) {
                memo[node->nodeID] = BoolExpr::createFalse();
            }
            else if (tbl.all1()) {
                memo[node->nodeID] = BoolExpr::createTrue();
            }
            else {
                // gather child expressions
                std::vector<std::shared_ptr<BoolExpr>> childF(k);
                for (uint32_t i = 0; i < k; ++i) {
                    childF[i] = memo.at(node->children[i].get()->nodeID);
                }

                // build DNF: one conjunction per true‐row
                std::vector<std::shared_ptr<BoolExpr>> terms;
                terms.reserve(rows);
                for (uint64_t m = 0; m < rows; ++m) {
                    if (!tbl.bits().bit(m))
                        continue;

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
                memo[node->nodeID] = mkOr(terms);
            }
        }
    }

    // root’s expression must be in memo
    return memo.at(root->nodeID);
}
