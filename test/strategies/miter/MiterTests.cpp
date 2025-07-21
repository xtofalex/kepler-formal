#include <gtest/gtest.h>
#include <string>
// SPDX-FileCopyrightText: 2024 The Naja authors
// <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include "gtest/gtest.h"

#include "ConstantPropagation.h"
#include "MiterStrategy.h"
#include "NLLibraryTruthTables.h"
#include "NLUniverse.h"
#include "NetlistGraph.h"
#include "SNLDesignTruthTable.h"
#include "SNLScalarNet.h"
#include "SNLScalarTerm.h"

using namespace naja;
using namespace naja::NL;
using namespace naja::NAJA_OPT;
using namespace KEPLER_FORMAL;

namespace {

void executeCommand(const std::string& command) {
  int result = system(command.c_str());
  if (result != 0) {
    std::cerr << "Command execution failed." << std::endl;
  }
}

}  // namespace

class MiterTests : public ::testing::Test {
 protected:
  MiterTests() {
    // You can do set-up work for each test here
  }
  ~MiterTests() override {
    // You can do clean-up work that doesn't throw exceptions here
  }
  void SetUp() override {
    // Code here will be called immediately after the constructor (right
    // before each test).
  }
  void TearDown() override {
    // Code here will be called immediately after each test (right
    // before the destructor).
    // Destroy the SNL
    NLUniverse::get()->destroy();
  }
};

TEST(HelloTest, ReturnsHelloWorld) {
  EXPECT_EQ(false, false);
}

TEST_F(MiterTests, TestMiterAND) {
  // 1. Create SNL
  NLUniverse* univ = NLUniverse::create();
  NLDB* db = NLDB::create(univ);
  NLLibrary* library =
      NLLibrary::create(db, NLLibrary::Type::Primitives, NLName("nangate45"));
  // 2. Create a top model with one output
  SNLDesign* top =
      SNLDesign::create(library, SNLDesign::Type::Primitive, NLName("top"));
  univ->setTopDesign(top);
  auto topOut =
      SNLScalarTerm::create(top, SNLTerm::Direction::Output, NLName("out"));
  auto topOut2 =
      SNLScalarTerm::create(top, SNLTerm::Direction::Output, NLName("out2"));
  // 3. create a logic_0 model
  SNLDesign* logic0 =
      SNLDesign::create(library, SNLDesign::Type::Primitive, NLName("LOGIC0"));
  // add output to logic0
  auto logic0Out =
      SNLScalarTerm::create(logic0, SNLTerm::Direction::Output, NLName("out"));
  // 4. create a logic_1 model
  SNLDesign* logic1 =
      SNLDesign::create(library, SNLDesign::Type::Primitive, NLName("LOGIC1"));
  // add output to logic0
  auto logic1Out =
      SNLScalarTerm::create(logic1, SNLTerm::Direction::Output, NLName("out"));
  SNLDesignTruthTable::setTruthTable(logic0, SNLTruthTable(0, 0));
  SNLDesignTruthTable::setTruthTable(logic1, SNLTruthTable(0, 1));
  NLLibraryTruthTables::construct(library);
  // 5. create a logic_0 instace in top
  SNLInstance* inst1 = SNLInstance::create(top, logic0, NLName("logic0"));
  // 6. create a logic_1 instace in top
  SNLInstance* inst2 = SNLInstance::create(top, logic1, NLName("logic1"));
  // 7. create a and model
  SNLDesign* andModel =
      SNLDesign::create(library, SNLDesign::Type::Primitive, NLName("AND"));

  // add 2 inputs and 1 output to and
  auto andIn1 =
      SNLScalarTerm::create(andModel, SNLTerm::Direction::Input, NLName("in1"));
  auto andIn2 =
      SNLScalarTerm::create(andModel, SNLTerm::Direction::Input, NLName("in2"));
  auto andOut = SNLScalarTerm::create(andModel, SNLTerm::Direction::Output,
                                      NLName("out"));
  // 8. create a and instance in top
  SNLInstance* inst3 = SNLInstance::create(top, andModel, NLName("and"));
  SNLInstance* inst4 = SNLInstance::create(top, andModel, NLName("and2"));
  // set truth table for and model
  SNLDesignTruthTable::setTruthTable(andModel, SNLTruthTable(2, 8));
  // 9. connect all instances inputs
  SNLNet* net1 = SNLScalarNet::create(top, NLName("logic_0_net"));
  net1->setType(SNLNet::Type::Assign0);
  SNLNet* net2 = SNLScalarNet::create(top, NLName("logic_1_net"));
  net2->setType(SNLNet::Type::Assign1);
  SNLNet* net3 = SNLScalarNet::create(top, NLName("and_output_net"));
  SNLNet* net4 = SNLScalarNet::create(top, NLName("and2_output_net"));
  // connect logic0 to and
  inst1->getInstTerm(logic0Out)->setNet(net1);

  inst4->getInstTerm(andIn1)->setNet(net2);
  inst4->getInstTerm(andIn2)->setNet(net2);
  // connect logic1 to and
  inst2->getInstTerm(logic1Out)->setNet(net2);
  inst3->getInstTerm(andIn2)->setNet(net1);
  inst3->getInstTerm(andIn1)->setNet(net4);
  // connect the and instance output to the top output
  inst3->getInstTerm(andOut)->setNet(net3);
  topOut->setNet(net3);
  inst4->getInstTerm(andOut)->setNet(net4);
  topOut2->setNet(net4);
  // 11. create DNL
  get();
  // 12. create a constant propagation object
  {
    std::string dotFileName(
        std::string(std::string("./beforeCP") + std::string(".dot")));
    std::string svgFileName(
        std::string(std::string("./beforeCP") + std::string(".svg")));
    SnlVisualiser snl(top);
    snl.process();
    snl.getNetlistGraph().dumpDotFile(dotFileName.c_str());
    executeCommand(std::string(std::string("dot -Tsvg ") + dotFileName +
                               std::string(" -o ") + svgFileName)
                       .c_str());
  }
  ConstantPropagation cp;
  // 13. collect the constants
  // cp.collectConstants();
  // 14. run the constant propagation
  {
    MiterStrategy miter;
    miter.build();
    for (const auto& po : miter.getPOs()) {
      std::cout << "PO: " << po.toString() << std::endl;
    }
  }

  cp.run();
  // 15. check the output value of the top instance
  {
    std::string dotFileName(
        std::string(std::string("./afterCP") + std::string(".dot")));
    std::string svgFileName(
        std::string(std::string("./afterCP") + std::string(".svg")));
    SnlVisualiser snl(top);
    snl.process();
    snl.getNetlistGraph().dumpDotFile(dotFileName.c_str());
    executeCommand(std::string(std::string("dot -Tsvg ") + dotFileName +
                               std::string(" -o ") + svgFileName)
                       .c_str());
  }
  {
    MiterStrategy miter;
    miter.build();
    for (const auto& po : miter.getPOs()) {
      std::cout << "PO: " << po.toString() << std::endl;
    }
  }
  naja::DNL::destroy();
}

TEST_F(MiterTests, TestMiterANDNonConstant) {
  // 1. Create SNL
  NLUniverse* univ = NLUniverse::create();
  NLDB* db = NLDB::create(univ);
  NLLibrary* library =
      NLLibrary::create(db, NLLibrary::Type::Primitives, NLName("nangate45"));
  // 2. Create a top model with one output
  SNLDesign* top =
      SNLDesign::create(library, SNLDesign::Type::Primitive, NLName("top"));
  univ->setTopDesign(top);
  auto topOut =
      SNLScalarTerm::create(top, SNLTerm::Direction::Output, NLName("out"));
  auto topOut2 =
      SNLScalarTerm::create(top, SNLTerm::Direction::Output, NLName("out2"));
    auto topIn1 =
      SNLScalarTerm::create(top, SNLTerm::Direction::Input, NLName("In1"));
  auto topIn2 =
      SNLScalarTerm::create(top, SNLTerm::Direction::Input, NLName("In2"));
  NLLibraryTruthTables::construct(library);
  // 7. create a and model
  SNLDesign* andModel =
      SNLDesign::create(library, SNLDesign::Type::Primitive, NLName("AND"));

  // add 2 inputs and 1 output to and
  auto andIn1 =
      SNLScalarTerm::create(andModel, SNLTerm::Direction::Input, NLName("in1"));
  auto andIn2 =
      SNLScalarTerm::create(andModel, SNLTerm::Direction::Input, NLName("in2"));
  auto andOut = SNLScalarTerm::create(andModel, SNLTerm::Direction::Output,
                                      NLName("out"));
  // 8. create a and instance in top
  SNLInstance* inst3 = SNLInstance::create(top, andModel, NLName("and"));
  SNLInstance* inst4 = SNLInstance::create(top, andModel, NLName("and2"));
  // set truth table for and model
  SNLDesignTruthTable::setTruthTable(andModel, SNLTruthTable(2, 8));
  // 9. connect all instances inputs
  SNLNet* net1 = SNLScalarNet::create(top, NLName("top_in1_net"));
  SNLNet* net2 = SNLScalarNet::create(top, NLName("top_in2_net"));
  SNLNet* net3 = SNLScalarNet::create(top, NLName("and_output_net"));
  SNLNet* net4 = SNLScalarNet::create(top, NLName("and2_output_net"));
  // connect logic0 to and
  topIn1->setNet(net1);

  inst4->getInstTerm(andIn1)->setNet(net2);
  inst4->getInstTerm(andIn2)->setNet(net2);
  // connect logic1 to and
  topIn2->setNet(net2);
  inst3->getInstTerm(andIn2)->setNet(net1);
  inst3->getInstTerm(andIn1)->setNet(net4);
  // connect the and instance output to the top output
  inst3->getInstTerm(andOut)->setNet(net3);
  topOut->setNet(net3);
  inst4->getInstTerm(andOut)->setNet(net4);
  topOut2->setNet(net4);
  // 11. create DNL
  get();
  // 12. create a constant propagation object
  {
    std::string dotFileName(
        std::string(std::string("./beforeCP") + std::string(".dot")));
    std::string svgFileName(
        std::string(std::string("./beforeCP") + std::string(".svg")));
    SnlVisualiser snl(top);
    snl.process();
    snl.getNetlistGraph().dumpDotFile(dotFileName.c_str());
    executeCommand(std::string(std::string("dot -Tsvg ") + dotFileName +
                               std::string(" -o ") + svgFileName)
                       .c_str());
  }
  ConstantPropagation cp;
  // 13. collect the constants
  // cp.collectConstants();
  // 14. run the constant propagation
  {
    MiterStrategy miter;
    miter.build();
    for (const auto& po : miter.getPOs()) {
      std::cout << "PO: " << po.toString() << std::endl;
    }
  }

  cp.run();
  // 15. check the output value of the top instance
  {
    std::string dotFileName(
        std::string(std::string("./afterCP") + std::string(".dot")));
    std::string svgFileName(
        std::string(std::string("./afterCP") + std::string(".svg")));
    SnlVisualiser snl(top);
    snl.process();
    snl.getNetlistGraph().dumpDotFile(dotFileName.c_str());
    executeCommand(std::string(std::string("dot -Tsvg ") + dotFileName +
                               std::string(" -o ") + svgFileName)
                       .c_str());
  }
  {
    MiterStrategy miter;
    miter.build();
    for (const auto& po : miter.getPOs()) {
      std::cout << "PO: " << po.toString() << std::endl;
    }
  }
  naja::DNL::destroy();
}

// Required main function for Google Test
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
