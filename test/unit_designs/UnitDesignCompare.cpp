#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>

#include "NLUniverse.h"

#include "SNLUtils.h"
#include "SNLVRLConstructor.h"
#include "MiterStrategy.h"

using namespace naja::NL;

#ifndef BENCHMARKS_PATH
#define BENCHMARKS_PATH "Undefined"
#endif

class UnitDesignCompare: public ::testing::Test {
  protected:
    void SetUp() override {
      NLUniverse* universe = NLUniverse::create();
      auto db = NLDB::create(universe);
      library0_ = NLLibrary::create(db, NLName("LIB0"));
      library1_ = NLLibrary::create(db, NLName("LIB1"));
    }
    void TearDown() override {
      NLUniverse::get()->destroy();
    }
  protected:
    NLLibrary*  library0_;
    NLLibrary*  library1_;
};

TEST_F(UnitDesignCompare, testSameDesigns) {
  SNLVRLConstructor constructor0(library0_);
  std::filesystem::path benchmarksPath(BENCHMARKS_PATH);
  constructor0.construct(benchmarksPath/"simple0.v");
  auto top = SNLUtils::findTop(library0_);

  KEPLER_FORMAL::MiterStrategy miterS(top, top);
  EXPECT_TRUE(miterS.run());
}

TEST_F(UnitDesignCompare, testDifferentDesigns) {
  std::filesystem::path benchmarksPath(BENCHMARKS_PATH);
  SNLVRLConstructor constructor0(library0_);
  constructor0.construct(benchmarksPath/"simple0.v");
  auto top0 = SNLUtils::findTop(library0_);

  SNLVRLConstructor constructor1(library1_);
  constructor1.construct(benchmarksPath/"simple1.v");
  auto top1 = SNLUtils::findTop(library1_);

  KEPLER_FORMAL::MiterStrategy miterS(top0, top1);
  EXPECT_FALSE(miterS.run());
  //should be different
  //here the issue comes from missing truth table but nothing is reported
}

TEST_F(UnitDesignCompare, testDiffWithConstants) {
  std::filesystem::path benchmarksPath(BENCHMARKS_PATH);
  SNLVRLConstructor constructor0(library0_);
  constructor0.construct(benchmarksPath/"simple1.v");
  auto top0 = SNLUtils::findTop(library0_);

  SNLVRLConstructor constructor1(library1_);
  constructor1.construct(benchmarksPath/"simple2.v");
  auto top1 = SNLUtils::findTop(library1_);

  KEPLER_FORMAL::MiterStrategy miterS(top0, top1);
  EXPECT_FALSE(miterS.run());
  //should be different
  //here the issue comes from missing truth table but nothing is reported
}