/*
 * OCLOperatorFactory.hpp
 *
 *  Created on: Mar 25, 2015
 *      Author: pfandedd
 */

#include <sgpp/base/operation/hash/OperationMultipleEval.hpp>
#include <sgpp/base/exception/factory_exception.hpp>
#include <sgpp/base/opencl/OpenCLConfigurationParameters.hpp>
#include <sgpp/globaldef.hpp>
#include "StreamingModOCLFastMultiPlatformOperatorFactory.hpp"

namespace SGPP {
  namespace datadriven {

    base::OperationMultipleEval* createStreamingModOCLFastMultiPlatformConfigured(base::Grid& grid, base::DataMatrix& dataset) {

      std::map<std::string, std::string> defaultParameter;
      defaultParameter["KERNEL_USE_LOCAL_MEMORY"] = "true";
      defaultParameter["KERNEL_DATA_BLOCKING_SIZE"] = "1";
      defaultParameter["LINEAR_LOAD_BALANCING_VERBOSE"] = "false";
      //  defaultParameter["KERNEL_GRID_BLOCK_SIZE"] = "1";
      defaultParameter["KERNEL_TRANS_DATA_BLOCK_SIZE"] = "1";
      defaultParameter["KERNEL_TRANS_UNROLL_1D"] = "true";
      defaultParameter["KERNEL_STORE_DATA"] = "array";

      base::OpenCLConfigurationParameters parameters("StreamingModOCLFastMultiPlatform.cfg", defaultParameter);

      std::cout << "are optimizations on: " << parameters.getAsBoolean("ENABLE_OPTIMIZATIONS") << std::endl;
      std::cout << "is local memory on: " << parameters.getAsBoolean("KERNEL_USE_LOCAL_MEMORY") << std::endl;
      std::cout << "local size: " << parameters.getAsUnsigned("LOCAL_SIZE") << std::endl;
      std::cout << "internal precision: " << parameters["INTERNAL_PRECISION"] << std::endl;
      std::cout << "platform is: " << parameters["PLATFORM"] << std::endl;
      std::cout << "device type is: " << parameters["DEVICE_TYPE"] << std::endl;

      if (parameters["INTERNAL_PRECISION"] == "float") {
        return new datadriven::OperationMultiEvalStreamingModOCLFastMultiPlatform<float>(grid, dataset, parameters);
      } else if (parameters["INTERNAL_PRECISION"] == "double") {
        return new datadriven::OperationMultiEvalStreamingModOCLFastMultiPlatform<double>(grid, dataset, parameters);
      } else {
        throw base::factory_exception(
          "Error creating operation\"OperationMultiEvalStreamingModOCL\": invalid value for parameter \"INTERNAL_PRECISION\"");
      }
    }

  }
}
