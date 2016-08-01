// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/parallel/datadriven/algorithm/AlgorithmAdaBoostVectorizedIdentity.hpp>
#include <sgpp/parallel/datadriven/algorithm/DMWeightMatrixVectorizedIdentity.hpp>
#include <sgpp/base/exception/operation_exception.hpp>
#include <sgpp/base/operation/BaseOpFactory.hpp>

#include <sgpp/globaldef.hpp>

namespace sgpp {
namespace parallel {

AlgorithmAdaBoostVectorizedIdentity::AlgorithmAdaBoostVectorizedIdentity(
    sgpp::base::Grid& SparseGrid, size_t gridType, int gridLevel, sgpp::base::DataMatrix& trainData,
    sgpp::base::DataVector& trainDataClass, size_t NUM, double lambda, size_t IMAX, double eps,
    size_t IMAX_final, double eps_final, double firstLabel, double secondLabel, double threshold,
    double maxLambda, double minLambda, size_t searchNum, bool refine, size_t refineMode,
    size_t refineNum, size_t numberOfAda, double percentOfAda, VectorizationType vecMode,
    size_t mode)
    : AlgorithmAdaBoostBase(SparseGrid, gridType,
                            static_cast<sgpp::base::level_t>(gridLevel), trainData,
                            trainDataClass, NUM, lambda, IMAX, eps, IMAX_final, eps_final,
                            firstLabel, secondLabel, threshold, maxLambda, minLambda, searchNum,
                            refine, refineMode, refineNum, numberOfAda, percentOfAda, mode) {
  if (vecMode != X86SIMD && vecMode != OpenCL && vecMode != ArBB &&
      vecMode != Hybrid_X86SIMD_OpenCL) {
    throw sgpp::base::operation_exception(
        "AlgorithmAdaBoostVectorizedIdentity : Only X86SIMD or OCL or ArBB or HYBRID_X86SIMD_OCL "
        "are supported vector extensions!");
  }

  this->vecMode = vecMode;
}

AlgorithmAdaBoostVectorizedIdentity::~AlgorithmAdaBoostVectorizedIdentity() {}

void AlgorithmAdaBoostVectorizedIdentity::alphaSolver(double& lambda,
                                                      sgpp::base::DataVector& weight,
                                                      sgpp::base::DataVector& alpha, bool final) {
  DMWeightMatrixVectorizedIdentity WMatrix(*this->grid, *this->data, lambda, weight, this->vecMode);
  sgpp::base::DataVector rhs(alpha.getSize());
  WMatrix.generateb(*this->classes, rhs);

  if (final) {
    sgpp::solver::ConjugateGradients myCG(this->imax_final, this->epsilon_final);
    myCG.solve(WMatrix, alpha, rhs, false, false, -1.0);
  } else {
    sgpp::solver::ConjugateGradients myCG(this->imax, this->epsilon);
    myCG.solve(WMatrix, alpha, rhs, false, false, -1.0);
  }
}

}  // namespace parallel
}  // namespace sgpp
