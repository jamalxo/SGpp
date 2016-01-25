// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef SGPP_OPTIMIZATION_FUNCTION_SCALAR_INTERPOLANTSCALARFUNCTIONHESSIAN_HPP
#define SGPP_OPTIMIZATION_FUNCTION_SCALAR_INTERPOLANTSCALARFUNCTIONHESSIAN_HPP

#include <sgpp/globaldef.hpp>

#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/base/datatypes/DataMatrix.hpp>
#include <sgpp/base/operation/hash/OperationNaiveEvalHessian.hpp>
#include <sgpp/base/operation/BaseOpFactory.hpp>
#include <sgpp/base/grid/Grid.hpp>
#include <sgpp/optimization/function/scalar/ScalarFunctionHessian.hpp>

namespace SGPP {
  namespace optimization {


    /**
     * Sparse grid interpolant Hessian of a scalar-valued function.
     *
     * @see InterpolantScalarFunction
     */
    class InterpolantScalarFunctionHessian : public ScalarFunctionHessian {
      public:
        /**
         * Constructor.
         * Do not destruct the grid before the
         * InterpolantScalarFunctionHessian object!
         *
         * @param grid  sparse grid
         * @param alpha coefficient vector
         */
        InterpolantScalarFunctionHessian(base::Grid& grid,
                                         const base::DataVector& alpha) :
          ScalarFunctionHessian(grid.getStorage()->dim()),
          grid(grid),
          opEvalHessian(op_factory::createOperationNaiveEvalHessian(grid)),
          alpha(alpha) {
        }

        /**
         * Destructor.
         */
        virtual ~InterpolantScalarFunctionHessian() override {
        }

        /**
         * Evaluation of the function, its gradient and its Hessian.
         *
         * @param      x        evaluation point \f$\vec{x} \in [0, 1]^d\f$
         * @param[out] gradient gradient
         *                      \f$\nabla f(\vec{x}) \in \mathbb{R}^d\f$
         * @param[out] hessian  Hessian matrix
         *                      \f$H_f(\vec{x}) \in \mathbb{R}^{d \times d}\f$
         * @return              \f$f(\vec{x})\f$
         */
        inline virtual float_t eval(const base::DataVector& x,
                                    base::DataVector& gradient,
                                    base::DataMatrix& hessian) override {
          for (size_t t = 0; t < d; t++) {
            if ((x[t] < 0.0) || (x[t] > 1.0)) {
              return INFINITY;
            }
          }

          return opEvalHessian->evalHessian(alpha, x, gradient, hessian);
        }

        /**
         * @param[out] clone pointer to cloned object
         */
        virtual void clone(std::unique_ptr<ScalarFunctionHessian>& clone) const override {
          clone = std::unique_ptr<ScalarFunctionHessian>(
                    new InterpolantScalarFunctionHessian(grid, alpha));
        }

        /**
         * @return coefficient vector
         */
        const base::DataVector& getAlpha() const {
          return alpha;
        }

        /**
         * @param alpha coefficient vector
         */
        void setAlpha(const base::DataVector& alpha) {
          this->alpha = alpha;
        }

      protected:
        /// sparse grid
        base::Grid& grid;
        /// pointer to evaluation operation
        std::unique_ptr<base::OperationNaiveEvalHessian> opEvalHessian;
        /// coefficient vector
        base::DataVector alpha;
    };

  }
}

#endif /* SGPP_OPTIMIZATION_FUNCTION_SCALAR_INTERPOLANTSCALARFUNCTIONHESSIAN_HPP */
