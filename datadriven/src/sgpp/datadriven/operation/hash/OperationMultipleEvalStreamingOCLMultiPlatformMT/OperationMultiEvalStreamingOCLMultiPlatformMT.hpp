// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <omp.h>
#include <chrono>

#include <sgpp/base/operation/hash/OperationMultipleEval.hpp>
#include <sgpp/base/tools/SGppStopwatch.hpp>
#include <sgpp/base/exception/operation_exception.hpp>
#include <sgpp/globaldef.hpp>
#include "../../../opencl/OCLConfigurationParameters.hpp"
#include "../../../opencl/OCLManager.hpp"
#include "StreamingOCLMultiPlatformMTKernelImpl.hpp"

namespace SGPP {
namespace datadriven {

template<typename T>
class OperationMultiEvalStreamingOCLMultiPlatformMT: public base::OperationMultipleEval {
protected:
    size_t dims;SGPP::base::DataMatrix preparedDataset;
    std::shared_ptr<base::OCLConfigurationParameters> parameters;
    std::vector<T> kernelDataset;
    size_t datasetSize = 0;
    // Member to store the sparse grid's levels for better vectorization
    std::vector<T> level;
    // Member to store the sparse grid's indices for better vectorization
    std::vector<T> index;
    size_t gridSize = 0;
    // Timer object to handle time measurements
    SGPP::base::SGppStopwatch myTimer;

    base::GridStorage* storage;

    float_t duration;

    std::shared_ptr<base::OCLManagerMultiPlatform> manager;
    std::unique_ptr<StreamingOCLMultiPlatformMTKernelImpl<T> > kernel;
public:

    OperationMultiEvalStreamingOCLMultiPlatformMT(base::Grid& grid, base::DataMatrix& dataset,
            std::shared_ptr<base::OCLConfigurationParameters> parameters) :
            OperationMultipleEval(grid, dataset), preparedDataset(dataset), parameters(parameters), myTimer(
            SGPP::base::SGppStopwatch()), duration(-1.0) {

        if (parameters->get("KERNEL_STORE_DATA").compare("register") == 0
                && dataset.getNcols() > parameters->getAsUnsigned("KERNEL_MAX_DIM_UNROLL")) {
            std::stringstream errorString;
            errorString
                    << "OCL Error: setting \"KERNEL_DATA_STORE\" to \"register\" requires value of \"KERNEL_MAX_DIM_UNROLL\"";
            errorString << " to be greater than the dimension of the data set" << std::endl;
            throw SGPP::base::operation_exception(errorString.str());
        }

        this->manager = std::make_shared<base::OCLManagerMultiPlatform>(parameters);

        this->dims = dataset.getNcols(); //be aware of transpose!
        this->kernel = std::make_unique<StreamingOCLMultiPlatformMTKernelImpl<T>>(dims, this->manager, parameters);

        this->storage = grid.getStorage();
        this->padDataset(this->preparedDataset);
        this->preparedDataset.transpose();
        this->datasetSize = this->preparedDataset.getNcols();

        //    std::cout << "dims: " << this->dims << std::endl;
        //    std::cout << "padded instances: " << this->datasetSize << std::endl;

        this->kernelDataset = std::vector<T>(this->preparedDataset.getNrows() * this->preparedDataset.getNcols());

        for (size_t i = 0; i < this->preparedDataset.getSize(); i++) {
            this->kernelDataset[i] = (T) this->preparedDataset[i];
        }

        //create the kernel specific data structures
        this->prepare();
    }

    ~OperationMultiEvalStreamingOCLMultiPlatformMT() {
    }

    void mult(SGPP::base::DataVector& alpha,
    SGPP::base::DataVector& result) override {
        this->myTimer.start();

        size_t gridFrom = 0;
        size_t gridTo = this->gridSize;
        size_t datasetFrom = 0;
        size_t datasetTo = this->datasetSize;

        std::vector<T>alphaArray(this->gridSize);

        for (size_t i = 0; i < alpha.getSize(); i++) {
            alphaArray[i] = (T) alpha[i];
        }

        for (size_t i = alpha.getSize(); i < this->gridSize; i++) {
            alphaArray[i] = 0.0;
        }

        std::vector<T>resultArray(this->datasetSize);

        for (size_t i = 0; i < this->datasetSize; i++) {
            resultArray[i] = 0.0;
        }

        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        this->kernel->mult(this->level, this->index, this->gridSize, this->kernelDataset, this->datasetSize, alphaArray,
                resultArray, gridFrom, gridTo, datasetFrom, datasetTo);
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;

        if (parameters->getAsBoolean("VERBOSE")) {
            std::cout << "duration mult streaming oclmp: " << elapsed_seconds.count() << std::endl;
        }

        for (size_t i = 0; i < result.getSize(); i++) {
            result[i] = resultArray[i];
        }

        this->duration = this->myTimer.stop();
    }

    void multTranspose(
    SGPP::base::DataVector& source,
    SGPP::base::DataVector& result) override {
        this->myTimer.start();

        size_t gridFrom = 0;
        size_t gridTo = this->gridSize;
        size_t datasetFrom = 0;
        size_t datasetTo = this->datasetSize;

        std::vector<T>sourceArray(this->datasetSize);

        for (size_t i = 0; i < source.getSize(); i++) {
            sourceArray[i] = (T) source[i];
        }

        for (size_t i = source.getSize(); i < this->datasetSize; i++) {
            sourceArray[i] = 0.0;
        }

        std::vector<T>resultArray(this->gridSize);

        for (size_t i = 0; i < this->gridSize; i++) {
            resultArray[i] = 0.0;
        }

        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        this->kernel->multTranspose(this->level, this->index, this->gridSize, this->kernelDataset,
                this->preparedDataset.getNcols(), sourceArray, resultArray, gridFrom, gridTo, datasetFrom, datasetTo);
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;

        if (parameters->getAsBoolean("VERBOSE")) {
            std::cout << "duration multTranspose streaming oclmp: " << elapsed_seconds.count() << std::endl;
        }

        for (size_t i = 0; i < result.getSize(); i++) {
            result[i] = resultArray[i];
        }

        this->duration = this->myTimer.stop();
    }

    float_t getDuration() override {
        return this->duration;
    }

    void prepare() override {
        this->recalculateLevelAndIndex();

        this->kernel->resetKernel();

        //    std::cout << "gridSize: " << this->gridSize << std::endl;
    }

private:

    size_t padDataset(
    SGPP::base::DataMatrix& dataset) {

        size_t dataBlocking = parameters->getAsUnsigned("KERNEL_DATA_BLOCKING_SIZE");
        size_t transGridBlocking = parameters->getAsUnsigned("KERNEL_TRANS_GRID_BLOCKING_SIZE");

        size_t blockingPadRequirements = std::max(dataBlocking, transGridBlocking);
        size_t vecWidth = parameters->getAsUnsigned("LOCAL_SIZE") * blockingPadRequirements;

        // Assure that data has a even number of instances -> padding might be needed
        size_t remainder = dataset.getNrows() % vecWidth;
        size_t loopCount = vecWidth - remainder;

        if (loopCount != vecWidth) {
            SGPP::base::DataVector lastRow(dataset.getNcols());
            size_t oldSize = dataset.getNrows();
            dataset.getRow(dataset.getNrows() - 1, lastRow);
            dataset.resize(dataset.getNrows() + loopCount);

            for (size_t i = 0; i < loopCount; i++) {
                dataset.setRow(oldSize + i, lastRow);
            }
        }

        return dataset.getNrows();
    }

    void recalculateLevelAndIndex() {

        size_t dataBlocking = parameters->getAsUnsigned("KERNEL_DATA_BLOCKING_SIZE");
        size_t transGridBlocking = parameters->getAsUnsigned("KERNEL_TRANS_GRID_BLOCKING_SIZE");

        size_t blockingPadRequirements = std::max(dataBlocking, transGridBlocking);

        uint32_t localWorkSize = static_cast<uint32_t>(parameters->getAsUnsigned("LOCAL_SIZE")) * static_cast<uint32_t>(blockingPadRequirements);

        size_t remainder = this->storage->size() % localWorkSize;
        size_t padding = 0;

        if (remainder != 0) {
            padding = localWorkSize - remainder;
        }

        this->gridSize = this->storage->size() + padding;

        SGPP::base::DataMatrix* levelMatrix = new SGPP::base::DataMatrix(this->gridSize, this->dims);
        SGPP::base::DataMatrix* indexMatrix = new SGPP::base::DataMatrix(this->gridSize, this->dims);

        this->storage->getLevelIndexArraysForEval(*levelMatrix, *indexMatrix);

        for (size_t i = this->storage->size(); i < this->gridSize; i++) {
            for (size_t j = 0; j < this->storage->dim(); j++) {
                levelMatrix->set(i, j, 1.0);
                indexMatrix->set(i, j, 1.0);
            }
        }

        this->level = std::vector<T>(this->gridSize * this->dims);
        this->index = std::vector<T>(this->gridSize * this->dims);

        for (size_t i = 0; i < this->gridSize * this->dims; i++) {
            this->level[i] = (T) (*levelMatrix)[i];
            this->index[i] = (T) (*indexMatrix)[i];
        }

        delete levelMatrix;
        delete indexMatrix;
    }
};

}
}