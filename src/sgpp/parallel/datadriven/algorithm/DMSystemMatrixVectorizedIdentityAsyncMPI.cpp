/******************************************************************************
* Copyright (C) 2010 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)
// @author Roman Karlstetter (karlstetter@mytum.de)

#include "parallel/tools/MPI/SGppMPITools.hpp"
#include "base/exception/operation_exception.hpp"

#include "parallel/datadriven/tools/DMVectorizationPaddingAssistant.hpp"
#include "parallel/datadriven/basis/linear/noboundary/operation/impl/X86SimdLinearMult.hpp"
#include "parallel/datadriven/basis/linear/noboundary/operation/impl/X86SimdLinearMultTranspose.hpp"
#include "parallel/datadriven/algorithm/DMSystemMatrixVectorizedIdentityAsyncMPI.hpp"
#include "parallel/operation/ParallelOpFactory.hpp"
#include "parallel/tools/PartitioningTool.hpp"
#include <omp.h>


#define APPROXCHUNKSIZEGRID 50 // approximately how many blocks should be computed for the grid before sending
#define APPROXCHUNKSIZEDATA 50 // approximately how many blocks should be computed for the data before sending

namespace sg
{
namespace parallel
{

DMSystemMatrixVectorizedIdentityAsyncMPI::DMSystemMatrixVectorizedIdentityAsyncMPI(sg::base::Grid& SparseGrid, sg::base::DataMatrix& trainData, double lambda, VectorizationType vecMode)
	: DMSystemMatrixBase(trainData, lambda), vecMode_(vecMode), numTrainingInstances_(0), numPatchedTrainingInstances_(0), m_grid(SparseGrid)
{
	// handle unsupported vector extensions
	if (this->vecMode_ != X86SIMD &&
			this->vecMode_ != MIC &&
			this->vecMode_ != Hybrid_X86SIMD_MIC &&
			this->vecMode_ != OpenCL &&
			this->vecMode_ != ArBB &&
			this->vecMode_ != Hybrid_X86SIMD_OpenCL)
	{
		throw new sg::base::operation_exception("DMSystemMatrixSPVectorizedIdentity : un-supported vector extension!");
	}

	this->dataset_ = new sg::base::DataMatrix(trainData);
	this->numTrainingInstances_ = this->dataset_->getNrows();
	this->numPatchedTrainingInstances_ = sg::parallel::DMVectorizationPaddingAssistant::padDataset(*(this->dataset_), vecMode_);

	if (this->vecMode_ != OpenCL && this->vecMode_ != ArBB && this->vecMode_ != Hybrid_X86SIMD_OpenCL)
	{
		this->dataset_->transpose();
	}

	int mpi_size = sg::parallel::myGlobalMPIComm->getNumRanks();
	int thread_count = 1;
#ifdef _OPENMP
#pragma omp parallel
	{
		thread_count = omp_get_num_threads();
	}
#endif

	size_t blockCountData = this->numPatchedTrainingInstances_/sg::parallel::DMVectorizationPaddingAssistant::getVecWidth(this->vecMode_); // this process has (in total) blockCountData blocks of Data to process
	_chunkCountData = blockCountData/APPROXCHUNKSIZEDATA;

	// arrays for distribution settings
	_mpi_data_sizes = new int[_chunkCountData];
	_mpi_data_offsets = new int[_chunkCountData];
	_mpi_data_sizes_global = new int[mpi_size];
	_mpi_data_offsets_global = new int[mpi_size];

	sg::parallel::PartitioningTool::calcDistribution(this->numPatchedTrainingInstances_, _chunkCountData, _mpi_data_sizes, _mpi_data_offsets, sg::parallel::DMVectorizationPaddingAssistant::getVecWidth(this->vecMode_));
	sg::parallel::PartitioningTool::calcDistribution(_chunkCountData, mpi_size, _mpi_data_sizes_global, _mpi_data_offsets_global, 1);

	_chunkCountGrid = m_grid.getSize()/APPROXCHUNKSIZEGRID;

	// arrays for distribution settings
	_mpi_grid_sizes = new int[_chunkCountGrid];
	_mpi_grid_offsets = new int[_chunkCountGrid];
	_mpi_grid_sizes_global = new int[mpi_size];
	_mpi_grid_offsets_global = new int[mpi_size];

	sg::parallel::PartitioningTool::calcDistribution(m_grid.getSize(), _chunkCountGrid, _mpi_grid_sizes, _mpi_grid_offsets, 1);
	sg::parallel::PartitioningTool::calcDistribution(_chunkCountGrid, mpi_size, _mpi_grid_sizes_global, _mpi_grid_offsets_global, 1);


	this->level_ = new sg::base::DataMatrix(m_grid.getSize(), m_grid.getStorage()->dim());
	this->index_ = new sg::base::DataMatrix(m_grid.getSize(), m_grid.getStorage()->dim());

	m_grid.getStorage()->getLevelIndexArraysForEval(*(this->level_), *(this->index_));

	// mult: distribute calculations over dataset
	// multTranspose: distribute calculations over grid
}

DMSystemMatrixVectorizedIdentityAsyncMPI::~DMSystemMatrixVectorizedIdentityAsyncMPI()
{
	delete this->dataset_;

	delete this->level_;
	delete this->index_;

	delete[] this->_mpi_grid_sizes;
	delete[] this->_mpi_grid_offsets;
	delete[] this->_mpi_data_sizes;
	delete[] this->_mpi_data_offsets;

	delete[] this->_mpi_grid_sizes_global;
	delete[] this->_mpi_grid_offsets_global;
	delete[] this->_mpi_data_sizes_global;
	delete[] this->_mpi_data_offsets_global;
}

void DMSystemMatrixVectorizedIdentityAsyncMPI::mult(sg::base::DataVector& alpha, sg::base::DataVector& result)
{
	sg::base::DataVector temp(this->numPatchedTrainingInstances_);
	result.setAll(0.0);
	double* ptrResult = result.getPointer();
	double* ptrTemp = temp.getPointer();

	int mpi_size = sg::parallel::myGlobalMPIComm->getNumRanks();
	int mpi_myrank = sg::parallel::myGlobalMPIComm->getMyRank();


	MPI_Request dataRecvReqs[_chunkCountData]; //allocating a little more than necessary, otherwise complicated index computations needed
	int tagsData[_chunkCountData];
	for (int i = 0; i<_chunkCountData; i++){
		tagsData[i] = _mpi_data_offsets[i]*2 + 2;
	}
	sg::parallel::myGlobalMPIComm->IrecvFromAll(ptrTemp, _mpi_data_sizes_global, _mpi_data_offsets_global, _mpi_data_sizes, _mpi_data_offsets, tagsData, _chunkCountData, dataRecvReqs);

	MPI_Request gridRecvReqs[_chunkCountGrid]; //allocating a little more than necessary, otherwise complicated index computations needed
	int tagsGrid[_chunkCountGrid];
	for (int i = 0; i<_chunkCountGrid; i++){
		tagsGrid[i] = _mpi_grid_offsets[i]*2 + 3;
	}
	sg::parallel::myGlobalMPIComm->IrecvFromAll(ptrResult, _mpi_grid_sizes_global, _mpi_grid_offsets_global, _mpi_grid_sizes, _mpi_grid_offsets, tagsGrid, _chunkCountGrid, gridRecvReqs);

	this->myTimer_->start();
	int thread_count = 1;
	int thread_num = 0;
	#pragma omp parallel private (thread_num, thread_count)
	{
#ifdef _OPENMP
		thread_count = omp_get_num_threads();
		thread_num = omp_get_thread_num();
#endif

		size_t myDataChunkStart = _mpi_data_offsets_global[mpi_myrank];
		size_t myDataChunkEnd = myDataChunkStart + _mpi_data_sizes_global[mpi_myrank];
		for(size_t thread_chunk = myDataChunkStart + thread_num; thread_chunk<myDataChunkEnd; thread_chunk+=thread_count){
			size_t start = _mpi_data_offsets[thread_chunk];
			size_t end =  start + _mpi_data_sizes[thread_chunk];

			if (start % sg::parallel::X86SimdLinearMult::getChunkDataPoints() != 0 || end % sg::parallel::X86SimdLinearMult::getChunkDataPoints() != 0)
			{
				std::cout << "start%CHUNKDATAPOINTS_X86: " << start%sg::parallel::X86SimdLinearMult::getChunkDataPoints() << "; end%CHUNKDATAPOINTS_X86: " << end%sg::parallel::X86SimdLinearMult::getChunkDataPoints() << std::endl;
				throw sg::base::operation_exception("processed vector segment must fit to CHUNKDATAPOINTS_X86!");
			}

			sg::parallel::X86SimdLinearMult::mult(level_, index_, dataset_, alpha, temp, 0, alpha.getSize(), start, end);

			sg::parallel::myGlobalMPIComm->IsendToAll(&ptrTemp[start], _mpi_data_sizes[thread_chunk], start*2 + 2);

		}
	#pragma omp single
		{
			this->computeTimeMult_ += this->myTimer_->stop();

			MPI_Waitall(_chunkCountData, dataRecvReqs, MPI_STATUSES_IGNORE);
			this->completeTimeMult_ += this->myTimer_->stop();

			// patch result -> set additional entries zero
			if (this->numTrainingInstances_ != temp.getSize())
			{
				for (size_t i = 0; i < (temp.getSize()-this->numTrainingInstances_); i++)
				{
					temp.set(temp.getSize()-(i+1), 0.0f);
				}
			}


			this->myTimer_->start();

		} //implicit OpenMP barrier here

		size_t myGridChunkStart = _mpi_grid_offsets_global[mpi_myrank];
		size_t myGridChunkEnd = myGridChunkStart + _mpi_grid_sizes_global[mpi_myrank];

		for(size_t thread_chunk = myGridChunkStart + thread_num; thread_chunk<myGridChunkEnd; thread_chunk+=thread_count){
			size_t start = _mpi_grid_offsets[thread_chunk];
			size_t end =  start + _mpi_grid_sizes[thread_chunk];

			sg::parallel::X86SimdLinearMultTranspose::multTranspose(level_, index_, dataset_, temp, result, start, end, 0, this->numPatchedTrainingInstances_);
			sg::parallel::myGlobalMPIComm->IsendToAll(&ptrResult[start], _mpi_grid_sizes[thread_chunk], start*2 + 3);
		}


	}
	this->computeTimeMultTrans_ += this->myTimer_->stop();

	MPI_Waitall(_chunkCountGrid, gridRecvReqs, MPI_STATUSES_IGNORE);
	this->completeTimeMultTrans_ += this->myTimer_->stop();

	result.axpy(static_cast<double>(this->numTrainingInstances_)*this->lambda_, alpha);
	if (mpi_myrank == 0) std::cout << "*";
}

void DMSystemMatrixVectorizedIdentityAsyncMPI::generateb(sg::base::DataVector& classes, sg::base::DataVector& b)
{
	int mpi_size = sg::parallel::myGlobalMPIComm->getNumRanks();
	int mpi_myrank = sg::parallel::myGlobalMPIComm->getMyRank();

	double* ptrB = b.getPointer();
	b.setAll(0.0);

	sg::base::DataVector myClasses(classes);
	// Apply padding
	if (this->numPatchedTrainingInstances_ != myClasses.getSize())
	{
		myClasses.resizeZero(this->numPatchedTrainingInstances_);
	}

	MPI_Request gridRecvReqs[_chunkCountGrid]; //allocating a little more than necessary, otherwise complicated index computations needed
	int tags[_chunkCountGrid];
	for(int i = 0; i<_chunkCountGrid; i++){
		tags[i] = _mpi_grid_offsets[i]+1;
	}
	sg::parallel::myGlobalMPIComm->IrecvFromAll(ptrB, _mpi_grid_sizes_global, _mpi_grid_offsets_global, _mpi_grid_sizes, _mpi_grid_offsets, tags, _chunkCountGrid, gridRecvReqs);

	int thread_count = 1;
	int thread_num = 0;
#pragma omp parallel private(thread_count, thread_num)
	{
#ifdef _OPENMP
		thread_count = omp_get_num_threads();
		thread_num = omp_get_thread_num();
#endif
		size_t myGridChunkStart = _mpi_grid_offsets_global[mpi_myrank];
		size_t myGridChunkEnd = myGridChunkStart + _mpi_grid_sizes_global[mpi_myrank];
		for(size_t thread_chunk = myGridChunkStart + thread_num; thread_chunk<myGridChunkEnd; thread_chunk+=thread_count){
			size_t start = _mpi_grid_offsets[thread_chunk];
			size_t end =  start + _mpi_grid_sizes[thread_chunk];

			sg::parallel::X86SimdLinearMultTranspose::multTranspose(level_, index_, dataset_, myClasses, b, start, end, 0, this->numPatchedTrainingInstances_);
			//std::cout << "alltoall[" << thread_chunk << "]: from " << start << "; size" << _mpi_grid_sizes[thread_chunk] <<   std::endl;
			sg::parallel::myGlobalMPIComm->IsendToAll(&ptrB[start], _mpi_grid_sizes[thread_chunk], start+1);
			//std::cout << "after isendtoall " << thread_chunk << " --- " << _mpi_grid_offsets[thread_chunk] <<  std::endl;
		}
	}
	MPI_Status stats[_chunkCountGrid];
	MPI_Waitall(_chunkCountGrid, gridRecvReqs, stats);

	for(int i= 0; i<_chunkCountGrid; i++) {
		if(stats[i].MPI_ERROR != MPI_SUCCESS){
			std::cout << "["<< mpi_myrank <<"] status error at index "<< i << ": " << stats[i].MPI_ERROR << std::endl;
			throw new sg::base::operation_exception("DMSystemMatrixSPVectorizedIdentity : status error!");
		}
	}
}

void DMSystemMatrixVectorizedIdentityAsyncMPI::rebuildLevelAndIndex()
{
	delete this->level_;
	delete this->index_;

	this->level_ = new sg::base::DataMatrix(m_grid.getSize(), m_grid.getStorage()->dim());
	this->index_ = new sg::base::DataMatrix(m_grid.getSize(), m_grid.getStorage()->dim());

	m_grid.getStorage()->getLevelIndexArraysForEval(*(this->level_), *(this->index_));

	_chunkCountGrid = m_grid.getSize()/APPROXCHUNKSIZEGRID;

	delete[] _mpi_grid_sizes;
	delete[] _mpi_grid_offsets;

	// arrays for distribution settings
	_mpi_grid_sizes = new int[_chunkCountGrid];
	_mpi_grid_offsets = new int[_chunkCountGrid];

	sg::parallel::PartitioningTool::calcDistribution(m_grid.getSize(), _chunkCountGrid, _mpi_grid_sizes, _mpi_grid_offsets, 1);
	sg::parallel::PartitioningTool::calcDistribution(_chunkCountGrid, sg::parallel::myGlobalMPIComm->getNumRanks(), _mpi_grid_sizes_global, _mpi_grid_offsets_global, 1);

	if(sg::parallel::myGlobalMPIComm->getMyRank() == 0) {
		for(int i = 0; i<sg::parallel::myGlobalMPIComm->getNumRanks(); i++){
			//std::cout << "glob: " << _mpi_grid_offsets_global[i] << " (" << _mpi_grid_sizes_global[i] << ")" <<std::endl;
		}
		for (int i = 0; i<_chunkCountGrid; i++){
			//std::cout << "loc: " << _mpi_grid_offsets[i] << " (" << _mpi_grid_sizes[i] << ")" << std::endl;
		}
	}
}

}
}
