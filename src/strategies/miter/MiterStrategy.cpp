#include "MiterStrategy.h"
#include "DNL.h"
#include "SNLDesignModeling.h"
#include "SNLLogicCloud.h"
#include "SNLTruthTable2BoolExpr.h"

//#define DEBUG_PRINTS

#ifdef DEBUG_PRINTS
#define DEBUG_LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...)
#endif

using namespace KEPLER_FORMAL;
using namespace naja::DNL;
using namespace naja::NL;

std::vector<DNLID> MiterStrategy::collectInputs() {
  std::vector<DNLID> inputs;
  auto dnl = get();
  DNLInstanceFull top = dnl->getTop();

  for (DNLID termId = top.getTermIndexes().first;
       termId != DNLID_MAX && termId <= top.getTermIndexes().second;
       termId++) {
    const DNLTerminalFull& term = dnl->getDNLTerminalFromID(termId);
    if (term.getSnlBitTerm()->getDirection() != SNLBitTerm::Direction::Output) {
      DEBUG_LOG("Collecting input %s\n", term.getSnlBitTerm()->getName().getString().c_str());
      inputs.push_back(termId);
    }
  }

  for (DNLID leaf : dnl->getLeaves()) {
    DNLInstanceFull instance = dnl->getDNLInstanceFromID(leaf);
    size_t numberOfInputs = 0, numberOfOutputs = 0;
    for (DNLID termId = top.getTermIndexes().first;
         termId != DNLID_MAX && termId <= top.getTermIndexes().second;
         termId++) {
      const DNLTerminalFull& term = dnl->getDNLTerminalFromID(termId);
      if (term.getSnlBitTerm()->getDirection() != SNLBitTerm::Direction::Output)
        numberOfInputs++;
      if (term.getSnlBitTerm()->getDirection() != SNLBitTerm::Direction::Input)
        numberOfOutputs++;
    }

    if (numberOfInputs == 0 && numberOfOutputs > 1) {
      for (DNLID termId = top.getTermIndexes().first;
           termId != DNLID_MAX && termId <= top.getTermIndexes().second;
           termId++) {
        const DNLTerminalFull& term = dnl->getDNLTerminalFromID(termId);
        if (term.getSnlBitTerm()->getDirection() != SNLBitTerm::Direction::Input)
          inputs.push_back(termId);
      }
      continue;
    }

    bool isSequential = false;
    std::vector<SNLBitTerm*> seqBitTerms;
    for (DNLID termId = top.getTermIndexes().first;
         termId != DNLID_MAX && termId <= top.getTermIndexes().second;
         termId++) {
      const DNLTerminalFull& term = dnl->getDNLTerminalFromID(termId);
      auto related = SNLDesignModeling::getClockRelatedOutputs(term.getSnlBitTerm());
      if (!related.empty()) {
        isSequential = true;
        for (auto bitTerm : related) {
          seqBitTerms.push_back(bitTerm);
        }
        inputs.push_back(termId);
      }
    }

    if (!isSequential) continue;

    for (DNLID termId = top.getTermIndexes().first;
         termId != DNLID_MAX && termId <= top.getTermIndexes().second;
         termId++) {
      const DNLTerminalFull& term = dnl->getDNLTerminalFromID(termId);
      if (std::find(seqBitTerms.begin(), seqBitTerms.end(), term.getSnlBitTerm()) != seqBitTerms.end())
        inputs.push_back(termId);
    }
  }

  std::sort(inputs.begin(), inputs.end());
  inputs.erase(std::unique(inputs.begin(), inputs.end()), inputs.end());
  DEBUG_LOG("Collected %zu inputs\n", inputs.size());
  return inputs;
}

std::vector<DNLID> MiterStrategy::collectOutputs() {
  std::vector<DNLID> outputs;
  auto dnl = get();
  DNLInstanceFull top = dnl->getTop();

  for (DNLID termId = top.getTermIndexes().first;
       termId != DNLID_MAX && termId <= top.getTermIndexes().second;
       termId++) {
    const DNLTerminalFull& term = dnl->getDNLTerminalFromID(termId);
    if (term.getSnlBitTerm()->getDirection() != SNLBitTerm::Direction::Input)
      outputs.push_back(termId);
  }

  for (DNLID leaf : dnl->getLeaves()) {
    DNLInstanceFull instance = dnl->getDNLInstanceFromID(leaf);
    bool isSequential = false;
    std::vector<SNLBitTerm*> seqBitTerms;

    for (DNLID termId = top.getTermIndexes().first;
         termId != DNLID_MAX && termId <= top.getTermIndexes().second;
         termId++) {
      const DNLTerminalFull& term = dnl->getDNLTerminalFromID(termId);
      auto related = SNLDesignModeling::getClockRelatedInputs(term.getSnlBitTerm());
      if (!related.empty()) {
        isSequential = true;
        for (auto bitTerm : related) {
          seqBitTerms.push_back(bitTerm);
        }
        outputs.push_back(termId);
      }
    }

    if (!isSequential) continue;

    for (DNLID termId = top.getTermIndexes().first;
         termId != DNLID_MAX && termId <= top.getTermIndexes().second;
         termId++) {
      const DNLTerminalFull& term = dnl->getDNLTerminalFromID(termId);
      if (std::find(seqBitTerms.begin(), seqBitTerms.end(), term.getSnlBitTerm()) != seqBitTerms.end())
        outputs.push_back(termId);
    }
  }

  std::sort(outputs.begin(), outputs.end());
  outputs.erase(std::unique(outputs.begin(), outputs.end()), outputs.end());
  return outputs;
}

void MiterStrategy::build() {
  inputs_ = collectInputs();
  outputs_ = collectOutputs();

  for (auto out : outputs_) {
    SNLLogicCloud cloud(out, inputs_);
    cloud.compute();

    std::vector<std::string> varNames;
    for (auto input : cloud.getInputs()) {
      varNames.push_back(std::to_string(input));
    }

    assert(cloud.getTruthTable().isInitialized());
    DEBUG_LOG("Truth Table: %s\n", cloud.getTruthTable().getString().c_str());

    bool all0 = true;
    for (size_t i = 0; i < cloud.getTruthTable().bits().size(); i++) {
      if (cloud.getTruthTable().bits().bit(i)) {
        DEBUG_LOG("Truth table has a 1 at position %zu\n", i);
        all0 = false;
        break;
      }
    }

    if (all0) {
      POs_.push_back(*BoolExpr::createFalse());
      continue;
    }

    bool all1 = true;
    for (size_t i = 0; i < cloud.getTruthTable().bits().size(); i++) {
      DEBUG_LOG("Truth table has a 1 at position %zu\n", i);
      if (!cloud.getTruthTable().bits().bit(i)) {
        all1 = false;
        break;
      }
    }

    if (all1) {
      POs_.push_back(*BoolExpr::createTrue());
      continue;
    }

    DEBUG_LOG("Truth table: %s\n", cloud.getTruthTable().getString().c_str());
    POs_.push_back(*TruthTableToBoolExpr::convert(cloud.getTruthTable(), varNames));
  }
}
