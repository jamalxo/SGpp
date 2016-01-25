/*
 * kernelTuner.cpp
 *
 *  Created on: Oct 27, 2015
 *      Author: pfandedd
 */
#include <iostream>

#if USE_OCL == 1
#include <boost/program_options.hpp>
#include "sgpp/datadriven/application/MetaLearner.hpp"
#include "sgpp/datadriven/operation/hash/simple/DatadrivenOperationCommon.hpp"
#include <sgpp/base/opencl/OCLOperationConfiguration.hpp>
#include <sgpp/datadriven/application/StaticParameterTuner.hpp>
#include <sgpp/datadriven/application/LearnerScenario.hpp>

int main(int argc, char** argv) {

  std::string scenarioFileName;
  std::string parameterConfigurationFileName; // <- includes kernel type, subtype
  std::string kernelName;
  std::string outputFileName;
  bool collectStatistics = false;
  bool useDoublePrecision = true;
  //    std::vector<std::string> devices;

  boost::program_options::options_description description("Allowed options");
  description.add_options()("help", "display help")("scenario",
      boost::program_options::value<std::string>(&scenarioFileName),
      "the scenario file to be used (serialized LearnerScenario)")("parameterConfiguration",
          boost::program_options::value<std::string>(&parameterConfigurationFileName),
          "the parameter configuration file to be used (serialized StaticParameterTuner)")("kernel",
              boost::program_options::value<std::string>(&kernelName), "name of the kernel to be tuned for")(
                "outputFileName", boost::program_options::value<std::string>(&outputFileName),
                "output file for optimized parameters")("collectStatistics",
                    boost::program_options::value<bool>(&collectStatistics),
                    "collect statistics for each device and kernel optimized and write them to csv files")("useDoublePrecision",
                        boost::program_options::value<bool>(&useDoublePrecision),
                        "tune for double precision")
  //            ("devices",
  //            boost::program_options::value<std::vector<std::string> >(&devices)->multitoken(),
  //            "specify comma-separated list of devices or \"all\" for all devices found")
  ;

  boost::program_options::variables_map variables_map;

  boost::program_options::parsed_options options = parse_command_line(argc, argv, description);
  boost::program_options::store(options, variables_map);
  boost::program_options::notify(variables_map);

  if (variables_map.count("help")) {
    std::cout << description << std::endl;
    return 0;
  }

  //check whether all files exist
  std::ifstream scenarioFile(scenarioFileName);

  if (!scenarioFile.good()) {
    std::cout << "scenario file not found" << std::endl;
    return 1;
  }

  scenarioFile.close();
  std::ifstream parameterConfigurationFile(parameterConfigurationFileName);

  if (!parameterConfigurationFile.good()) {
    std::cout << "parameter file not found" << std::endl;
    return 1;
  }

  parameterConfigurationFile.close();

  SGPP::datadriven::LearnerScenario scenario(scenarioFileName);

  SGPP::base::OCLOperationConfiguration parameter(parameterConfigurationFileName);

  SGPP::datadriven::StaticParameterTuner staticParameterTuner(parameter, true, true);

  //TODO: LOCAL_SIZE should be added
  if (kernelName.compare("StreamingOCLMultiPlatform") == 0) {
    staticParameterTuner.addParameter("KERNEL_USE_LOCAL_MEMORY", { "false", "true" }); //
    staticParameterTuner.addParameter("KERNEL_DATA_BLOCKING_SIZE", { "1", "2", "4", "8" }); //
    staticParameterTuner.addParameter("KERNEL_TRANS_GRID_BLOCKING_SIZE", { "1", "2", "4", "8" }); //
    staticParameterTuner.addParameter("KERNEL_STORE_DATA", { "register", "array" }); //
    staticParameterTuner.addParameter("KERNEL_MAX_DIM_UNROLL", { "1", "4", }); // "8", "16"
  } else if (kernelName.compare("StreamingModOCLFastMultiPlatform") == 0) {
    staticParameterTuner.addParameter("KERNEL_USE_LOCAL_MEMORY", { "false", "true" }); //
    staticParameterTuner.addParameter("KERNEL_DATA_BLOCKING_SIZE", { "1", "2", "4", "8" }); //
    staticParameterTuner.addParameter("KERNEL_TRANS_DATA_BLOCK_SIZE", { "1", "2", "4", "8" }); //
    staticParameterTuner.addParameter("KERNEL_TRANS_GRID_BLOCK_SIZE", { "1", "4", "2", "4", "8" }); //
    staticParameterTuner.addParameter("KERNEL_STORE_DATA", { "register", "array" }); //
    staticParameterTuner.addParameter("KERNEL_MAX_DIM_UNROLL", { "1", "4", }); // "8", "16"
  } else {
    throw;
  }

  SGPP::base::OCLOperationConfiguration bestParameters = staticParameterTuner.tuneEverything(scenario, kernelName, useDoublePrecision);

  bestParameters.serialize(outputFileName);

  std::cout << "-------------- all done! --------------" << std::endl;
  return 0;
}
#else
int main(int argc, char** argv) {
  std::cout << "no OpenCL support" << std::endl;
}
#endif

