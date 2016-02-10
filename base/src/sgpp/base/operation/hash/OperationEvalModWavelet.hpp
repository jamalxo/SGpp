// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef OPERATIONEVALMODWAVELET_HPP
#define OPERATIONEVALMODWAVELET_HPP

#include <sgpp/base/operation/hash/OperationEval.hpp>
#include <sgpp/base/grid/GridStorage.hpp>

#include <sgpp/globaldef.hpp>


namespace SGPP {
namespace base {

/**
 * This class implements OperationEval for a grids with mod wavelet basis ansatzfunctions with
 */
class OperationEvalModWavelet : public OperationEval {
 public:
  /**
   * Constructor
   *
   * @param storage the grid's GridStorage object
   */
  explicit OperationEvalModWavelet(GridStorage* storage) : storage(storage) {}

  /**
   * Destructor
   */
  ~OperationEvalModWavelet() override {}

  float_t eval(const DataVector& alpha,
               const DataVector& point) override;

 protected:
  /// Pointer to GridStorage object
  GridStorage* storage;
};

}  // namespace base
}  // namespace SGPP

#endif /* OPERATIINEVALMODWAVELET_HPP */
