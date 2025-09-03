#include "SNLTruthTableTree.h"
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <cassert>

using namespace naja::NL;
using namespace KEPLER_FORMAL;

//----------------------------------------------------------------------
// Node::eval
//----------------------------------------------------------------------
bool SNLTruthTableTree::Node::eval(const std::vector<bool>& extInputs) const {
  if (type == Type::Input) {
    if (inputIndex >= extInputs.size())
      throw std::out_of_range("InputNode: index out of range");
    return extInputs[inputIndex];
  }
  // Table node:
  uint32_t arity = table.size();
  if (children.size() != arity)
    throw std::logic_error("TableNode: children count mismatch");

  // collect child results
  std::vector<bool> args;
  args.reserve(arity);
  for (auto const& c : children)
    args.push_back(c->eval(extInputs));

  // build index
  uint32_t idx = 0;
  for (uint32_t i = 0; i < arity; ++i)
    if (args[i])
      idx |= (1u << i);

  return table.bits().bit(idx);
}

//----------------------------------------------------------------------
// Tree ctors
//----------------------------------------------------------------------
SNLTruthTableTree::SNLTruthTableTree()
  : root_(nullptr), numExternalInputs_(0) {}

SNLTruthTableTree::SNLTruthTableTree(SNLTruthTable table) {
  // create table‐node root with inputleaves [0..arity−1]
  uint32_t arity = table.size();
  auto node = std::make_unique<Node>(std::move(table), this);
  for (uint32_t i = 0; i < arity; ++i)
    node->addChild(std::make_unique<Node>(i, this));
  root_ = std::move(node);
  numExternalInputs_ = arity;
  updateBorderLeaves();
}

SNLTruthTableTree::SNLTruthTableTree(std::unique_ptr<Node> root,
                                     size_t numExternalInputs)
  : root_(std::move(root))
  , numExternalInputs_(numExternalInputs)
{
  updateBorderLeaves();
}

//----------------------------------------------------------------------
// size / eval
//----------------------------------------------------------------------
size_t SNLTruthTableTree::size() const {
  return numExternalInputs_;
}

bool SNLTruthTableTree::eval(const std::vector<bool>& extInputs) const {
  if (!root_ || extInputs.size() != numExternalInputs_)
    throw std::invalid_argument("wrong input size or uninitialized tree");
  return root_->eval(extInputs);
}

//----------------------------------------------------------------------
// updateBorderLeaves (collect free input leaves)
//----------------------------------------------------------------------
void SNLTruthTableTree::updateBorderLeaves() {
  borderLeaves_.clear();
  struct Frame { Node* parent; size_t childPos; Node* node; };
  std::vector<Frame> stk;
  stk.reserve(64);
  stk.push_back({nullptr, 0, root_.get()});

  while (!stk.empty()) {
    auto f = stk.back(); stk.pop_back();
    if (f.node->type == Node::Type::Table) {
      // push children (reverse so index order is preserved)
      for (size_t i = f.node->children.size(); i-- > 0;) {
        stk.push_back({f.node, i, f.node->children[i].get()});
      }
    }
    else {
      // input leaf
      borderLeaves_.push_back({f.parent, f.childPos, f.node->inputIndex});
    }
  }

  std::sort(borderLeaves_.begin(), borderLeaves_.end(),
            [](auto const& a, auto const& b){
              return a.extIndex < b.extIndex;
            });
}

//----------------------------------------------------------------------
// core graft logic (no safety checks here)
//----------------------------------------------------------------------
void SNLTruthTableTree::concatBody(size_t borderIndex,
                                   SNLTruthTable table)
{
    // 1) locate the leaf‐slot index
    size_t idx = borderIndex;
    if (idx >= borderLeaves_.size()) {
        bool found = false;
        for (size_t i = 0; i < borderLeaves_.size(); ++i) {
            if (borderLeaves_[i].extIndex == borderIndex) {
                idx = i;
                found = true;
                break;
            }
        }
        if (!found)
            throw std::out_of_range("concat: leafIndex out of range");
    }
    auto leaf = borderLeaves_[idx];

    // 2) detach the old leaf node (or root if no parent)
    std::unique_ptr<Node> oldLeaf;
    if (leaf.parent) {
        oldLeaf = std::move(leaf.parent->children[leaf.childPos]);
    }
    else {
        oldLeaf = std::move(root_);
    }

    // 3) create the new Table‐node
    uint32_t arity = table.size();  
    auto newNode = std::make_unique<Node>(std::move(table), this);

    // 4) only if arity>0 do we re‐use the old leaf + add new inputs
    if (arity > 0) {
        // child 0 = the old leaf we just removed
        newNode->addChild(std::move(oldLeaf));

        // children 1…arity-1 = fresh Input leaves
        for (uint32_t i = 1; i < arity; ++i) {
            newNode->addChild(
                std::make_unique<Node>(numExternalInputs_ + (i - 1), this)
            );
        }
    }
    // if arity==0, we leave newNode->children empty

    // 5) reattach into the tree
    if (leaf.parent) {
        leaf.parent->children[leaf.childPos] = std::move(newNode);
    }
    else {
        root_ = std::move(newNode);
    }
}


//----------------------------------------------------------------------
// public concat APIs
//----------------------------------------------------------------------
void SNLTruthTableTree::concat(size_t borderIndex, SNLTruthTable table) {
  concatBody(borderIndex, std::move(table));
  numExternalInputs_ += (table.size() - 1);
  updateBorderLeaves();
}

void SNLTruthTableTree::concatFull(std::vector<SNLTruthTable> tables) {
  int newInputs = (int) numExternalInputs_;
  if (tables.size() > borderLeaves_.size())
    throw std::invalid_argument("too many tables in concatFull");

  // splice in order
  
  for (size_t i = 0; i < tables.size(); ++i) {
    concatBody(i, std::move(tables[i]));
    newInputs += (((int) tables[i].size()) - 1);
  }
  numExternalInputs_ = (size_t) newInputs;
  updateBorderLeaves();
}

//----------------------------------------------------------------------
// isInitialized (walk and check every table)
//----------------------------------------------------------------------
bool SNLTruthTableTree::isInitialized() const {
  if (!root_) return false;
  struct Frame { Node* node; };
  std::vector<Frame> stk{{root_.get()}};

  while (!stk.empty()) {
    auto f = stk.back(); stk.pop_back();
    if (f.node->type == Node::Type::Table) {
      if (!f.node->table.isInitialized())
        return false;
      for (auto& c : f.node->children)
        stk.push_back({c.get()});
    }
  }
  return true;
}

//----------------------------------------------------------------------
// print (skeleton only; you can fill in your formatting)
//----------------------------------------------------------------------
void SNLTruthTableTree::print() const {
  if (!root_) return;
  struct Frame { Node* node; size_t depth; };
  std::vector<Frame> stk{{root_.get(), 0}};

  while (!stk.empty()) {
    auto [n, d] = stk.back(); stk.pop_back();
    // you can print indent (d), then type etc.
    if (n->type == Node::Type::Table) {
      for (size_t i = n->children.size(); i-- > 0;) {
        stk.push_back({n->children[i].get(), d+1});
      }
    }
  }
}
