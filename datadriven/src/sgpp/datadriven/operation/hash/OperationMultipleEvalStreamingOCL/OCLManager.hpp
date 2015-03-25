/*
 * OCLManager.hpp
 *
 *  Created on: Mar 12, 2015
 *      Author: pfandedd
 */

#pragma once

//define required for clCreateCommandQueue on platforms that don't support OCL2.0 yet
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>

#include <sgpp/base/tools/ConfigurationParameters.hpp>
#include <sgpp/base/exception/operation_exception.hpp>
#include "OCLKernelSourceBuilder.hpp"
#include "StreamingOCLParameters.hpp"

namespace SGPP {
namespace datadriven {

class OCLManager {
public:
	base::ConfigurationParameters parameters;
	cl_int err;
	cl_uint num_platforms;
	cl_platform_id platform_id;
	cl_platform_id* platform_ids;
	cl_device_id* device_ids;
	cl_uint num_devices;
	cl_command_queue* command_queue;
	cl_context context;

public:
	OCLManager(base::ConfigurationParameters parameters): parameters(parameters) {
		// read number of OpenCL devices environment variable: SGPP_NUM_OCL_DEVICES
		const char* num_ocl_devices_env = getenv("SGPP_NUM_OCL_DEVICES");
		unsigned int max_number_ocl_devices =
				std::numeric_limits<unsigned int>::max();

		if (num_ocl_devices_env != nullptr) {
			unsigned int num_ocl_devices_limit = (unsigned int) (strtoul(
					num_ocl_devices_env, nullptr, 0));

			if (num_ocl_devices_limit != 0) {
				max_number_ocl_devices = std::min<unsigned int>(
						max_number_ocl_devices, num_ocl_devices_limit);
			} else {
				std::cout << "Ignoring value: \"" << num_ocl_devices_env
						<< "\" for SGPP_NUM_OCL_DEVICES" << std::endl;
			}
		} else {
			max_number_ocl_devices = 8;
		}

		// determine number of available OpenCL platforms
		err = clGetPlatformIDs(0, nullptr, &num_platforms);

		if (err != CL_SUCCESS) {
			std::cout
					<< "OCL Error: Unable to get number of OpenCL platforms. Error Code: "
					<< err << std::endl;
		}

		std::cout << "OCL Info: " << num_platforms
				<< " OpenCL Platforms have been found" << std::endl;

		// get available platforms
		platform_ids = new cl_platform_id[num_platforms];
		err = clGetPlatformIDs(num_platforms, platform_ids, nullptr);

		if (err != CL_SUCCESS) {
			std::cout << "OCL Error: Unable to get Platform ID. Error Code: "
					<< err << std::endl;
		}

		for (cl_uint ui = 0; ui < num_platforms; ui++) {
			char vendor_name[128] = { 0 };
			err = clGetPlatformInfo(platform_ids[ui], CL_PLATFORM_VENDOR,
					128 * sizeof(char), vendor_name, nullptr);

			if (CL_SUCCESS != err) {
				std::cout << "OCL Error: Can't get platform vendor!"
						<< std::endl;
			} else {
				if (vendor_name != nullptr) {
					std::cout << "OCL Info: Platform " << ui << " vendor name: "
							<< vendor_name << std::endl;
				}

				platform_id = platform_ids[ui];
			}
		}
		std::cout << std::endl;

		// Find out how many devices there are
		device_ids = new cl_device_id[max_number_ocl_devices];
#if STREAMING_OCL_DEVICE_TYPE == CL_DEVICE_TYPE_CPU
		std::cout << "OCL Info: looking for CPU device" << std::endl;
#elif STREAMING_OCL_DEVICE_TYPE == CL_DEVICE_TYPE_GPU
		std::cout << "OCL Info: looking for GPU device" << std::endl;
#else
		std::cout << "OCL Info: looking for device of unknown type" << std::endl;
#endif
		err = clGetDeviceIDs(platform_id, STREAMING_OCL_DEVICE_TYPE,
				max_number_ocl_devices, device_ids, &num_devices);

		if (err != CL_SUCCESS) {
			std::cout << "OCL Error: Unable to get Device ID. Error Code: "
					<< err << std::endl;
		}

		// num_devices = 1;

		std::cout << "OCL Info: " << num_devices
				<< " OpenCL devices have been found!" << std::endl;

		// allocate arrays
		command_queue = new cl_command_queue[num_devices];

		// Create OpenCL context
		context = clCreateContext(0, num_devices, device_ids, nullptr, nullptr,
				&err);

		if (err != CL_SUCCESS) {
			std::cout
					<< "OCL Error: Failed to create OpenCL context! Error Code: "
					<< err << std::endl;
		}

		// Creating the command queues
		for (size_t i = 0; i < num_devices; i++) {

			command_queue[i] = clCreateCommandQueue(context, device_ids[i],
			CL_QUEUE_PROFILING_ENABLE, &err);

			if (err != CL_SUCCESS) {
				std::cout
						<< "OCL Error: Failed to create command queue! Error Code: "
						<< err << std::endl;
			}
		}

		std::cout
				<< "OCL Info: Successfully initialized OpenCL (local workgroup size: "
				<< parameters.getAsUnsigned("STREAMING_OCL_LOCAL_SIZE") << ")" << std::endl << std::endl;
	}

	/**
	 * @brief buildKernel builds the program that is represented by @a program_src and creates @a num_devices kernel objects
	 * that are stored into the array @a kernel (must be already allocated with at least @a num_devices )
	 *
	 * @param program_src the source of the program to compile
	 * @param kernel_name name of the kernel function (in program_src) to create the kernel for
	 * @param context OpenCL context
	 * @param num_devices number of OpenCL devices
	 * @param device_ids array with device ids, necessary for displaying build info
	 * @param kernel already allocated array: the resulting kernels are put into this array, one for each device (=> at least num_devices entries)
	 * @return
	 */
	cl_int buildKernel(const std::string& program_src, const char* kernel_name,
			cl_context context, size_t num_devices, cl_device_id* device_ids,
			cl_kernel* kernel) {
		cl_int err;

		//std::cout << program_src << std::endl;

		// setting the program
		const char* kernel_src = program_src.c_str();
		cl_program program = clCreateProgramWithSource(context, 1, &kernel_src,
		NULL, &err);

		if (err != CL_SUCCESS) {
			std::cout << "OCL Error: Failed to create program! Error Code: "
					<< err << std::endl;
			throw SGPP::base::operation_exception(
					"OCL Error: Failed to create program!");
		}

		std::string build_opts;
#if STREAMING_OCL_ENABLE_OPTIMIZATIONS == true
		build_opts = "-cl-finite-math-only -cl-fast-relaxed-math "; // -O5  -cl-mad-enable -cl-denorms-are-zero -cl-no-signed-zeros -cl-unsafe-math-optimizations -cl-finite-math-only -cl-fast-relaxed-math
#else
		build_opts = "-cl-opt-disable -g ";
#endif

		// compiling the program
		err = clBuildProgram(program, 0, NULL, build_opts.c_str(), NULL, NULL);

		if (err != CL_SUCCESS) {
			std::cout << "OCL Error: OpenCL Build Error. Error Code: " << err
					<< std::endl;

			size_t len;
			char buffer[4096];

			// get the build log
			clGetProgramBuildInfo(program, device_ids[0], CL_PROGRAM_BUILD_LOG,
					sizeof(buffer), buffer, &len);
			std::cout << "--- Build Log ---" << std::endl << buffer
					<< std::endl;
			return err;
		}

		// creating the kernel
		for (size_t i = 0; i < num_devices; i++) {
			kernel[i] = clCreateKernel(program, kernel_name, &err);

			if (err != CL_SUCCESS) {
				std::cout << "OCL Error: Failed to create kernel! Error Code: "
						<< err << std::endl;
				return err;
			}
		}

		if (program) {
			clReleaseProgram(program);
		}

		return CL_SUCCESS;
	}

	uint32_t getOCLLocalSize() {
//		// read environment variable for local work group size: SGPP_OCL_LOCAL_SIZE
//		const char* ocl_local_size_env = getenv("SGPP_OCL_LOCAL_SIZE");
//
//		if (ocl_local_size_env != NULL) {
//			unsigned int num_ocl_devices_envvalue = (unsigned int) (strtoul(
//					ocl_local_size_env, NULL, 0));
//
//			if (num_ocl_devices_envvalue != 0) {
//				return num_ocl_devices_envvalue;
//			} else {
//				std::cout << "Ignoring value: \"" << ocl_local_size_env
//						<< "\" for SGPP_OCL_LOCAL_SIZE" << std::endl;
//			}
//		}
//
//		return 64;
		return parameters.getAsUnsigned("STREAMING_OCL_LOCAL_SIZE");
	}
};

}
}