
#include <iostream>
#include <fstream>
#include <filesystem>

#include <gtest/gtest.h>

#include "ppcdisasm/ppc-dis.hpp"

#include "ppc2cpp/data_flow/DfgVisualizer.hpp"

#include "ppc2cpp/common/endian.h"
#include "ppc2cpp/model/Project.hpp"
#include "ppc2cpp/control_flow/ControlFlowAnalysis.hpp"
#include "ppc2cpp/data_flow/DataFlowAnalysis.hpp"
#include "ppc2cpp/data_flow/FlowContext.hpp"

#include "test_helper.h"

using namespace ppc2cpp;
using namespace std;

TEST(ControlFlowAnalysisTest, kartActionCalc) {
  BasicBlock expectedBlocks[] = {{0, 10, ppc2cpp::INTERNAL}, {10, 10, ppc2cpp::EXTERNAL}, {11, 13, ppc2cpp::INTERNAL},
    {25, 32, ppc2cpp::INTERNAL}, {32, 32, ppc2cpp::RUNTIME}, {14, 15, ppc2cpp::INTERNAL}, {21, 22, ppc2cpp::INTERNAL},
    {22, 22, ppc2cpp::EXTERNAL}, {23, 24, ppc2cpp::INTERNAL}, {24, 24, ppc2cpp::EXTERNAL}, {16, 18, ppc2cpp::INTERNAL},
    {19, 20, ppc2cpp::INTERNAL}};

  filesystem::path test_path = filesystem::path(TEST_PATH) / "binaries";
  // mkw binaries. TODO: test on more convenient binaries
  vector<filesystem::path> files {test_path / "main.dol", test_path / "StaticR.rel"};

  ProjectCreationOptions options;
  options.projectName = "test_project";
  options.programLoaderType = persistence::LOADER_RVL;
  options.inputFiles = files;
  options.projectFile= "./test.ppc2cpp";
  Project testProject = Project::createProject(options);

  ControlFlowAnalysis cfa(testProject.programLoader);
  // KartActionRealLocal::calc
  Function kartActionCalc(testProject.programLoader->getLocation("StaticR.rel", ".text", 0x59B78).value(), 0x80569fb0-0x80569f2c, "calc");
  cfa.functionCFA(kartActionCalc);
  const ControlFlowGraph& cfg = kartActionCalc.cfg;
  
  for (const auto& block : expectedBlocks) {
    EXPECT_BLOCK(cfg, block);
  }
}

TEST(DataFlowAnalysisTest, quatmul) {
  filesystem::path test_path = filesystem::path(TEST_PATH) / "binaries";
  // mkw binaries. TODO: test on more convenient binaries
  vector<filesystem::path> files {test_path / "main.dol", test_path / "StaticR.rel"};

  ProjectCreationOptions options;
  options.projectName = "test_project";
  options.programLoaderType = persistence::LOADER_RVL;
  options.inputFiles = files;
  options.projectFile= "./test.ppc2cpp";
  Project testProject = Project::createProject(options);

  Function quatmul(testProject.programLoader->getLocation("main.dol", ".text2", 0x8023a540-0x800072c0).value(), 0x8023a5c4-0x8023a540, "quatmul");
  ControlFlowAnalysis cfa(testProject.programLoader);
  cfa.functionCFA(quatmul);
  
  DataFlowAnalysis dfa(testProject.programLoader);
  dfa.functionDFA(quatmul);

  outputDfgDot(std::cout, testProject.programLoader, quatmul);
  //EXPECT_FALSE(true);
}

TEST(DataFlowAnalysisTest, canHop) {
  filesystem::path test_path = filesystem::path(TEST_PATH) / "binaries";
  // mkw binaries. TODO: test on more convenient binaries
  vector<filesystem::path> files {test_path / "main.dol", test_path / "StaticR.rel"};

  ProjectCreationOptions options;
  options.projectName = "test_project";
  options.programLoaderType = persistence::LOADER_RVL;
  options.inputFiles = files;
  options.projectFile= "./test.ppc2cpp";
  Project testProject = Project::createProject(options);

  Function kartActionCalc(testProject.programLoader->getLocation("StaticR.rel", ".text", 0x8057da18-0x805103b4).value(), 0x8057da5c-0x8057da18, "canHop");
  ControlFlowAnalysis cfa(testProject.programLoader);
  cfa.functionCFA(kartActionCalc);
  
  DataFlowAnalysis dfa(testProject.programLoader);
  dfa.functionDFA(kartActionCalc);

  outputDfgDot(std::cout, testProject.programLoader, kartActionCalc);
  //EXPECT_FALSE(true);
}

TEST(DataFlowAnalysisTest, canAirtimeHop) {
  filesystem::path test_path = filesystem::path(TEST_PATH) / "binaries";
  // mkw binaries. TODO: test on more convenient binaries
  vector<filesystem::path> files {test_path / "main.dol", test_path / "StaticR.rel"};

  ProjectCreationOptions options;
  options.projectName = "test_project";
  options.programLoaderType = persistence::LOADER_RVL;
  options.inputFiles = files;
  options.projectFile= "./test.ppc2cpp";
  Project testProject = Project::createProject(options);

  Function airtimeHop(testProject.programLoader->getLocation("StaticR.rel", ".text", 0x8057d888-0x805103b4).value(), 0x8057da18-0x8057d888, "canHop");
  ControlFlowAnalysis cfa(testProject.programLoader);
  cfa.functionCFA(airtimeHop);
  
  DataFlowAnalysis dfa(testProject.programLoader);
  dfa.functionDFA(airtimeHop);

  outputDfgDot(std::cout, testProject.programLoader, airtimeHop);
  EXPECT_FALSE(true);
}

TEST(DataFlowAnalysisTest, seAngleAxis) {
  filesystem::path test_path = filesystem::path(TEST_PATH) / "binaries";
  // mkw binaries. TODO: test on more convenient binaries
  vector<filesystem::path> files {test_path / "main.dol", test_path / "StaticR.rel"};

  ProjectCreationOptions options;
  options.projectName = "test_project";
  options.programLoaderType = persistence::LOADER_RVL;
  options.inputFiles = files;
  options.projectFile= "./test.ppc2cpp";
  Project testProject = Project::createProject(options);

  Function setAngleAxis(testProject.programLoader->getLocation("main.dol", ".text2", 0x8023a0a0-0x800072c0).value(), 0x8023a138-0x8023a0a0, "setAngleAxis");
  ControlFlowAnalysis cfa(testProject.programLoader);
  cfa.functionCFA(setAngleAxis);
  
  DataFlowAnalysis dfa(testProject.programLoader);
  dfa.functionDFA(setAngleAxis);

  outputDfgDot(std::cout, testProject.programLoader, setAngleAxis);
  //EXPECT_FALSE(true);
}