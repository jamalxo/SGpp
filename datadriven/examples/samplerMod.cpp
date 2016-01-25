#include <iostream>
#include <string>

#include "sgpp/datadriven/application/MetaLearner.hpp"
#include "sgpp/datadriven/operation/hash/simple/DatadrivenOperationCommon.hpp"

int main(int argc, char** argv) {

  int maxLevel = 9;
  //  int maxLevel = 2;

  //  std::string fileName = "debugging.arff";
  //   std::string fileName = "friedman_4d.arff";
  std::string fileName = "DR5_train.arff";
  //    std::string fileName = "chess_train.arff";
  //  std::string fileName = "chess_train.arff";
  //    std::string fileName = "bigger.arff";

  //SGPP::base::RegularGridConfiguration gridConfig;
  SGPP::base::RegularGridConfiguration gridConfig;
  SGPP::solver::SLESolverConfiguration SLESolverConfigRefine;
  SGPP::solver::SLESolverConfiguration SLESolverConfigFinal;
  SGPP::base::AdpativityConfiguration adaptConfig;

  // setup grid
  gridConfig.dim_ = 0; //dim is inferred from the data
  gridConfig.level_ = maxLevel;
  gridConfig.type_ = SGPP::base::GridType::ModLinear;

  // Set Adaptivity
  adaptConfig.maxLevelType_ = false;
  adaptConfig.noPoints_ = 80;
  adaptConfig.numRefinements_ = 0;
  adaptConfig.percent_ = 200.0;
  adaptConfig.threshold_ = 0.0;

  // Set solver during refinement
  SLESolverConfigRefine.eps_ = 0;
  SLESolverConfigRefine.maxIterations_ = 0;
  SLESolverConfigRefine.threshold_ = -1.0;
  SLESolverConfigRefine.type_ = SGPP::solver::SLESolverType::CG;

  // Set solver for final step
  SLESolverConfigFinal.eps_ = 0;
  SLESolverConfigFinal.maxIterations_ = 10;
  SLESolverConfigFinal.threshold_ = -1.0;
  SLESolverConfigFinal.type_ = SGPP::solver::SLESolverType::CG;

  std::string metaInformation = "refine: " + std::to_string((unsigned long long) adaptConfig.numRefinements_)
                                + " points: " + std::to_string((unsigned long long) adaptConfig.noPoints_) + " iterations: "
                                + std::to_string((unsigned long long) SLESolverConfigRefine.maxIterations_);

  bool verbose = true;
  SGPP::datadriven::MetaLearner learner(gridConfig, SLESolverConfigRefine, SLESolverConfigFinal, adaptConfig,
                                        verbose);

  //learner.learn(kernelType, fileName);
  //learner.learnReference(fileName);

  //buggy are:
  //subspace simple - 0
  //subspacelinear combined - 60
  //streaming default - 1600 (13 without avx)
  //streaming ocl - 13

  SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
    SGPP::datadriven::OperationMultipleEvalType::STREAMING,
    SGPP::datadriven::OperationMultipleEvalSubType::OCLMASK);

  double lambda = 0.000001;


  learner.learn(configuration, fileName, lambda);
  //learner.learnReference(fileName);

  //learner.learnAndTest(fileName, testFileName, isBinaryClassificationProblem);
  //learner.learnAndCompare(configuration, fileName, 8, 1.0);
  //learner.writeStatisticsFile("statistics.csv", "test");

  return EXIT_SUCCESS;
}
