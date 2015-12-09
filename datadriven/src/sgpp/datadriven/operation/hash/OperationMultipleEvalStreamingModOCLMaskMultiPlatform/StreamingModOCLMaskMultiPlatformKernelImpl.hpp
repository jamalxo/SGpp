// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include "StreamingModOCLMaskMultiPlatformKernelImpl.hpp"

#include <string.h>
#include <limits>
#include <chrono>
#include <CL/cl.h>
#include <omp.h>

#include <sgpp/globaldef.hpp>
#include <sgpp/base/opencl/LinearLoadBalancerMultiPlatform.hpp>
#include <sgpp/base/opencl/OCLClonedBufferSD.hpp>
#include <sgpp/base/opencl/OCLManagerMultiPlatform.hpp>
#include <sgpp/base/opencl/OCLStretchedBuffer.hpp>
#include "StreamingModOCLMaskMultiPlatformKernelSourceBuilder.hpp"

namespace SGPP {
namespace datadriven {

template<typename real_type>
class StreamingModOCLMaskMultiPlatformKernelImpl {
private:

    std::shared_ptr<base::OCLDevice> device;

    size_t dims;

    cl_int err;

    base::OCLClonedBufferSD<real_type> deviceLevel;
    base::OCLClonedBufferSD<real_type> deviceIndex;
    base::OCLClonedBufferSD<real_type> deviceMask;
    base::OCLClonedBufferSD<real_type> deviceOffset;
    base::OCLClonedBufferSD<real_type> deviceAlpha;

    base::OCLClonedBufferSD<real_type> deviceData;

    base::OCLClonedBufferSD<real_type> deviceResultData;

    base::OCLClonedBufferSD<real_type> deviceLevelTranspose;
    base::OCLClonedBufferSD<real_type> deviceIndexTranspose;
    base::OCLClonedBufferSD<real_type> deviceMaskTranspose;
    base::OCLClonedBufferSD<real_type> deviceOffsetTranspose;

    base::OCLClonedBufferSD<real_type> deviceDataTranspose;
    base::OCLClonedBufferSD<real_type> deviceSourceTranspose;

    base::OCLClonedBufferSD<real_type> deviceResultGridTranspose;

    cl_kernel kernelMult;
    cl_kernel kernelMultTranspose;

    double deviceTimingMult;
    double deviceTimingMultTranspose;

    StreamingModOCLMaskMultiPlatformKernelSourceBuilder kernelSourceBuilder;
    std::shared_ptr<base::OCLManagerMultiPlatform> manager;
    std::shared_ptr<base::OCLOperationConfiguration> parameters;

    std::shared_ptr<base::QueueLoadBalancer> queueLoadBalancerMult;
    std::shared_ptr<base::QueueLoadBalancer> queueLoadBalancerMultTranspose;

    bool verbose;

public:

    StreamingModOCLMaskMultiPlatformKernelImpl(std::shared_ptr<base::OCLDevice> device, size_t dims,
            std::shared_ptr<base::OCLManagerMultiPlatform> manager,
            std::shared_ptr<base::OCLOperationConfiguration> parameters,
            std::shared_ptr<base::QueueLoadBalancer> queueBalancerMult,
            std::shared_ptr<base::QueueLoadBalancer> queueBalancerMultTranpose) :
            device(device), dims(dims), err(CL_SUCCESS), deviceLevel(device), deviceIndex(device), deviceMask(device), deviceOffset(
                    device), deviceAlpha(device), deviceData(device), deviceResultData(device), deviceLevelTranspose(
                    device), deviceIndexTranspose(device), deviceMaskTranspose(device), deviceOffsetTranspose(device), deviceDataTranspose(
                    device), deviceSourceTranspose(device), deviceResultGridTranspose(device), kernelMult(nullptr), kernelMultTranspose(
                    nullptr), kernelSourceBuilder(parameters, dims), manager(manager), parameters(parameters), queueLoadBalancerMult(
                    queueBalancerMult), queueLoadBalancerMultTranspose(queueBalancerMultTranpose) {

        //initialize with same timing to enforce equal problem sizes in the beginning
        this->deviceTimingMult = 1.0;
        this->deviceTimingMultTranspose = 1.0;
        this->verbose = (*parameters)["VERBOSE"].getBool();
    }

    ~StreamingModOCLMaskMultiPlatformKernelImpl() {
        if (this->kernelMult != nullptr) {
            clReleaseKernel(this->kernelMult);
            this->kernelMult = nullptr;
        }

        if (this->kernelMultTranspose != nullptr) {
            clReleaseKernel(this->kernelMultTranspose);
            this->kernelMultTranspose = nullptr;
        }
    }

    void resetKernel() {
        //TODO: fix for splittedkernel -> currently won't work for multiple iterations
        //leads to a reallocation before next kernel execution
        releaseGridBuffers();
        releaseDataBuffers();
        releaseDatasetResultBuffer();

        releaseGridBuffersTranspose();
        releaseDataBuffersTranspose();
        releaseGridResultBufferTranspose();
    }

    double mult(std::vector<real_type> &level, std::vector<real_type> &index, std::vector<real_type> &mask,
            std::vector<real_type> &offset, std::vector<real_type> &dataset, std::vector<real_type> &alpha,
            std::vector<real_type> &result, const size_t start_index_grid, const size_t end_index_grid,
            const size_t start_index_data, const size_t end_index_data) {

        // check if there is something to do at all
        if (!(end_index_grid > start_index_grid && end_index_data > start_index_data)) {
            return 0.0;
        }

        if (this->kernelMult == nullptr) {
            std::string program_src = kernelSourceBuilder.generateSourceMult();
            this->kernelMult = manager->buildKernel(program_src, device, "multOCLMask");
        }

        if (!deviceLevel.isInitialized()) {
            initGridBuffers(level, index, mask, offset, alpha, start_index_grid, end_index_grid);
        }

        this->deviceTimingMult = 0.0;

        while (true) {

            size_t kernelStartData;
            size_t kernelEndData;

            // set kernel arguments
            size_t kernelStartGrid = start_index_grid;
            size_t kernelEndGrid = end_index_grid;

            //TODO: change after blocking is implemented
            //TODO: don't forget to set padding to DATA_BLOCKING * THREAD_BLOCK_SIZE
            size_t dataBlockingSize = 1;

            //TODO: start_index_data not considered!
            bool segmentAvailable = queueLoadBalancerMult->getNextSegment(kernelStartData, kernelEndData);
            if (!segmentAvailable) {
                break;
            }

            size_t rangeSizeUnblocked = kernelEndData - kernelStartData;

            if (verbose) {
                std::cout << "device: " << device->deviceId << " kernel from: " << kernelStartData << " to: "
                        << kernelEndData << " -> range: " << rangeSizeUnblocked << std::endl;
            }

            initDatasetBuffers(dataset, kernelStartData, kernelEndData);
            initDatasetResultBuffers(kernelStartData, kernelEndData);

            clFinish(device->commandQueue);
//            std::cout << "wrote to device: " << device->deviceId << "" << std::endl;

            size_t rangeSizeBlocked = (kernelEndData / dataBlockingSize) - (kernelStartData / dataBlockingSize);

            if (rangeSizeBlocked > 0) {

                err = clSetKernelArg(this->kernelMult, 0, sizeof(cl_mem), this->deviceLevel.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(this->kernelMult, 1, sizeof(cl_mem), this->deviceIndex.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(this->kernelMult, 2, sizeof(cl_mem), this->deviceMask.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(this->kernelMult, 3, sizeof(cl_mem), this->deviceOffset.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(this->kernelMult, 4, sizeof(cl_mem), this->deviceData.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(this->kernelMult, 5, sizeof(cl_mem), this->deviceAlpha.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(this->kernelMult, 6, sizeof(cl_mem), this->deviceResultData.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(this->kernelMult, 7, sizeof(cl_uint), &rangeSizeUnblocked); // resultsize == number of entries in dataset
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(this->kernelMult, 8, sizeof(cl_uint), &kernelStartGrid);
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(this->kernelMult, 9, sizeof(cl_uint), &kernelEndGrid);
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }

                cl_event clTiming = nullptr;

                // enqueue kernel
                size_t localSize = (*parameters)["LOCAL_SIZE"].getUInt();

//                std::cout << "commandQueue: " << device->commandQueue << std::endl;

                char deviceName[128] = { 0 };
                cl_uint err;
                err = clGetDeviceInfo(device->deviceId,
                CL_DEVICE_NAME, 128 * sizeof(char), &deviceName, nullptr);

                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to read the device name for device: " << device->deviceId
                            << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }

//                std::cout << "OCL Info: detected device, name: \"" << deviceName << "\"" << std::endl;

                err = clEnqueueNDRangeKernel(device->commandQueue, this->kernelMult, 1, 0, &rangeSizeUnblocked,
                        &localSize, 0, nullptr, &clTiming);

                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to enqueue kernel command! Error code: " << err << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }

                clFinish(device->commandQueue);
//                std::cout << "executed kernel: " << device->deviceId << "" << std::endl;

                //TODO: implement treatment of start_index_data in queueLoadBalancer!
                deviceResultData.readFromBuffer();

//                std::cout << "read from device: " << device->deviceId << "" << std::endl;
                clFinish(device->commandQueue);

                std::vector<real_type> &hostTemp = deviceResultData.getHostPointer();
                size_t deviceIndex = 0;
                for (size_t i = 0; i < rangeSizeUnblocked; i++) {
//                    std::cout << "resultDevice[" << deviceIndex << "] = " << hostTemp[deviceIndex] << std::endl;
                    result[kernelStartData + i] = hostTemp[deviceIndex];
                    deviceIndex += 1;
                }

                // determine kernel execution time
                cl_ulong startTime = 0;
                cl_ulong endTime = 0;

                err = clGetEventProfilingInfo(clTiming,
                CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, nullptr);

                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString
                            << "OCL Error: Failed to read start-time from command queue (or crash in mult)! Error code: "
                            << err << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }

                err = clGetEventProfilingInfo(clTiming,
                CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, nullptr);

                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to read end-time from command queue! Error code: " << err
                            << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }

                clReleaseEvent(clTiming);

                double time = 0.0;
                time = (double) (endTime - startTime);
                time *= 1e-9;

                if (verbose) {
                    std::cout << "device: " << device->deviceId << " duration: " << time << std::endl;
                }

                this->deviceTimingMult += time;
            }
        }

        return this->deviceTimingMult;
    }

    double multTranspose(std::vector<real_type> &level, std::vector<real_type> &index, std::vector<real_type> &mask,
            std::vector<real_type> &offset, std::vector<real_type> &dataset, std::vector<real_type> &source,
            std::vector<real_type> &result, const size_t start_index_grid, const size_t end_index_grid,
            const size_t start_index_data, const size_t end_index_data) {

        // check if there is something to do at all
        if (!(end_index_grid > start_index_grid && end_index_data > start_index_data)) {
            return 0.0;
        }

        if (this->kernelMultTranspose == nullptr) {
            std::string program_src = kernelSourceBuilder.generateSourceMultTrans();
            this->kernelMultTranspose = manager->buildKernel(program_src, device, "multTransOCLMask");
        }

        this->deviceTimingMultTranspose = 0.0;

        while (true) {

            size_t kernelStartData = start_index_data;
            size_t kernelEndData = end_index_data;

            // set kernel arguments
            size_t kernelStartGrid;
            size_t kernelEndGrid;

            //TODO: change after blocking is implemented
            //TODO: don't forget to set padding to DATA_BLOCKING * THREAD_BLOCK_SIZE
            size_t dataBlockingSize = 1;

            //TODO: start_index_data not considered!
            bool segmentAvailable = queueLoadBalancerMultTranspose->getNextSegment(kernelStartGrid, kernelEndGrid);
            if (!segmentAvailable) {
                break;
            }

            size_t rangeSizeUnblocked = kernelEndGrid - kernelStartGrid;

            if (verbose) {
                std::cout << "device: " << device->platformId << " kernel from: " << kernelStartGrid << " to: "
                        << kernelEndGrid << " -> range: " << rangeSizeUnblocked << std::endl;
            }


            initGridBuffersTranspose(level, index, mask, offset, kernelStartGrid, kernelEndGrid);
            initDatasetBuffersTranspose(dataset, source, kernelStartData, kernelEndData);
            initGridResultBuffersTranspose(kernelStartGrid, kernelEndGrid);

            clFinish(device->commandQueue);
//            std::cout << "wrote to device: " << device->deviceId << "" << std::endl;

            size_t rangeSizeBlocked = (kernelEndGrid / dataBlockingSize) - (kernelStartGrid / dataBlockingSize);

            if (rangeSizeBlocked > 0) {
                err = clSetKernelArg(kernelMultTranspose, 0, sizeof(cl_mem), this->deviceLevelTranspose.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(kernelMultTranspose, 1, sizeof(cl_mem), this->deviceIndexTranspose.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(kernelMultTranspose, 2, sizeof(cl_mem), this->deviceMaskTranspose.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(kernelMultTranspose, 3, sizeof(cl_mem), this->deviceOffsetTranspose.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(kernelMultTranspose, 4, sizeof(cl_mem), this->deviceDataTranspose.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(kernelMultTranspose, 5, sizeof(cl_mem), this->deviceSourceTranspose.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(kernelMultTranspose, 6, sizeof(cl_mem),
                        this->deviceResultGridTranspose.getBuffer());
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
//                err = clSetKernelArg(kernelMultTranspose, 7, sizeof(cl_uint), &rangeSizeUnblocked); // sourceSize == number of entries in dataset
//                if (err != CL_SUCCESS) {
//                    std::stringstream errorString;
//                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
//                    throw SGPP::base::operation_exception(errorString.str());
//                }
                err = clSetKernelArg(kernelMultTranspose, 7, sizeof(cl_uint), &kernelStartData);
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }
                err = clSetKernelArg(kernelMultTranspose, 8, sizeof(cl_uint), &kernelEndData);
                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to create kernel arguments for device " << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }

                cl_event clTiming;

                // enqueue kernels
                size_t local = (*parameters)["LOCAL_SIZE"].getUInt();

                err = clEnqueueNDRangeKernel(device->commandQueue, kernelMultTranspose, 1, 0, &rangeSizeBlocked, &local,
                        0, nullptr, &clTiming);

                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to enqueue kernel command! Error code: " << err << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }

                clFinish(device->commandQueue);
//                std::cout << "executed kernel: " << device->deviceId << "" << std::endl;

                deviceResultGridTranspose.readFromBuffer();

                clFinish(device->commandQueue);
//                std::cout << "read from buffer: " << device->deviceId << "" << std::endl;

                std::vector<real_type> &deviceResultGridTransposeHost = deviceResultGridTranspose.getHostPointer();
                for (size_t i = 0; i < rangeSizeUnblocked; i++) {
//                    std::cout << "resultDevice[" << i << "] = " << deviceResultGridTransposeHost[i] << std::endl;
//                    std::cout << "-> result[" << kernelStartGrid + i << "]" << std::endl;
                    result[kernelStartGrid + i] = deviceResultGridTransposeHost[i];
                }

                // determine kernel execution time
                cl_ulong startTime = 0;
                cl_ulong endTime = 0;

                err = clGetEventProfilingInfo(clTiming,
                CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, nullptr);

                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString
                            << "OCL Error: Failed to read start-time from command queue (or crash in multTranspose)! Error code: "
                            << err << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }

                err = clGetEventProfilingInfo(clTiming,
                CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, nullptr);

                if (err != CL_SUCCESS) {
                    std::stringstream errorString;
                    errorString << "OCL Error: Failed to read end-time from command queue! Error code: " << err
                            << std::endl;
                    throw SGPP::base::operation_exception(errorString.str());
                }

                double time = 0.0;
                time = (double) (endTime - startTime);
                time *= 1e-9;

                if (verbose) {
                    std::cout << "device: " << device->deviceId << " duration: " << time << std::endl;
                }

                this->deviceTimingMultTranspose += time;

                clReleaseEvent(clTiming);
            }
        }
        return this->deviceTimingMultTranspose;
    }
private:

    void releaseGridBuffers() {
        this->deviceLevel.freeBuffer();
        this->deviceIndex.freeBuffer();
        this->deviceMask.freeBuffer();
        this->deviceOffset.freeBuffer();
        this->deviceAlpha.freeBuffer();
    }

    void releaseDataBuffers() {
        this->deviceData.freeBuffer();
        this->deviceResultData.freeBuffer();
    }

    void releaseDatasetResultBuffer() {
        this->deviceResultData.freeBuffer();
    }

    void releaseGridBuffersTranspose() {
        this->deviceLevelTranspose.freeBuffer();
        this->deviceIndexTranspose.freeBuffer();
        this->deviceMaskTranspose.freeBuffer();
        this->deviceOffsetTranspose.freeBuffer();
    }

    void releaseDataBuffersTranspose() {
        this->deviceSourceTranspose.freeBuffer();
        this->deviceDataTranspose.freeBuffer();
    }

    void releaseGridResultBufferTranspose() {
        this->deviceResultGridTranspose.freeBuffer();
    }

    void initGridBuffers(std::vector<real_type> &level, std::vector<real_type> &index, std::vector<real_type> &mask,
            std::vector<real_type> &offset, std::vector<real_type> &alpha, size_t kernelStartGrid,
            size_t kernelEndGrid) {

        deviceLevel.intializeTo(level, dims, kernelStartGrid, kernelEndGrid);
        deviceIndex.intializeTo(index, dims, kernelStartGrid, kernelEndGrid);
        deviceMask.intializeTo(mask, dims, kernelStartGrid, kernelEndGrid);
        deviceOffset.intializeTo(offset, dims, kernelStartGrid, kernelEndGrid);
        deviceAlpha.intializeTo(alpha, 1, kernelStartGrid, kernelEndGrid);
    }

    void initDatasetBuffers(std::vector<real_type> &dataset, size_t kernelStartData, size_t kernelEndData) {

        deviceData.intializeTo(dataset, dims, kernelStartData, kernelEndData, true);
    }

    void initDatasetResultBuffers(size_t kernelStartData, size_t kernelEndData) {

        size_t range = kernelEndData - kernelStartData;

        std::vector<real_type> zeros(range);
        for (size_t i = 0; i < range; i++) {
            zeros[i] = 0.0;
        }

        deviceResultData.intializeTo(zeros, 1, 0, range);
    }

    void initGridBuffersTranspose(std::vector<real_type> &level, std::vector<real_type> &index,
            std::vector<real_type> &mask, std::vector<real_type> &offset, size_t kernelStartGrid,
            size_t kernelEndGrid) {


        deviceLevelTranspose.intializeTo(level, dims, kernelStartGrid, kernelEndGrid);
        deviceIndexTranspose.intializeTo(index, dims, kernelStartGrid, kernelEndGrid);
        deviceMaskTranspose.intializeTo(mask, dims, kernelStartGrid, kernelEndGrid);
        deviceOffsetTranspose.intializeTo(offset, dims, kernelStartGrid, kernelEndGrid);
    }

    void initDatasetBuffersTranspose(std::vector<real_type> &dataset, std::vector<real_type> &source,
            size_t kernelStartData, size_t kernelEndData) {

        deviceDataTranspose.intializeTo(dataset, dims, kernelStartData, kernelEndData, true);
        deviceSourceTranspose.intializeTo(source, 1, kernelStartData, kernelEndData);
    }

    void initGridResultBuffersTranspose(size_t kernelStartGrid, size_t kernelEndGrid) {

        size_t range = kernelEndGrid - kernelStartGrid;

        std::vector<real_type> zeros(range);
        for (size_t i = 0; i < range; i++) {
            zeros[i] = 0.0;
        }

        deviceResultGridTranspose.intializeTo(zeros, 1, 0, range);
    }

}
;

}
}
