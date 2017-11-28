// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef SGPP_OPTIMIZATION_FUNCTION_SCALAR_SCALEDSCALARFUNCTION_HPP
#define SGPP_OPTIMIZATION_FUNCTION_SCALAR_SCALEDSCALARFUNCTION_HPP

#include <sgpp/globaldef.hpp>
#include <sgpp/optimization/function/scalar/ScalarFunction.hpp>

namespace sgpp {
namespace optimization {

class ScaledScalarFunction : public ScalarFunction {
 public:
  explicit ScaledScalarFunction(const ScalarFunction& fOrig) :
    ScaledScalarFunction(
        fOrig,
        base::DataVector(fOrig.getNumberOfParameters()),
        base::DataVector(fOrig.getNumberOfParameters()),
        1.0) {}

  ScaledScalarFunction(const ScalarFunction& fOrig,
                       const base::DataVector& lowerBounds,
                       const base::DataVector& upperBounds,
                       double valueFactor) :
    ScalarFunction(fOrig.getNumberOfParameters()),
    lowerBounds(lowerBounds),
    upperBounds(upperBounds),
    valueFactor(valueFactor),
    xScaled(base::DataVector(d)) {
    fOrig.clone(this->fOrig);
  }

  ~ScaledScalarFunction() override {}

  inline double eval(const base::DataVector& x) override {
    // scale x from restricted domain
    for (size_t t = 0; t < d; t++) {
      xScaled[t] = lowerBounds[t] + x[t] * (upperBounds[t] - lowerBounds[t]);
    }

    // multiply with valueFactor
    return valueFactor * fOrig->eval(xScaled);
  }

  void clone(std::unique_ptr<ScalarFunction>& clone) const override {
    clone = std::unique_ptr<ScalarFunction>(
        new ScaledScalarFunction(*fOrig, lowerBounds, upperBounds, valueFactor));
  }

  const base::DataVector& getLowerBounds() const { return lowerBounds; }
  void setLowerBounds(const base::DataVector& lowerBounds) { this->lowerBounds = lowerBounds; }

  const base::DataVector& getUpperBounds() const { return upperBounds; }
  void setUpperBounds(const base::DataVector& upperBounds) { this->upperBounds = upperBounds; }

  double getValueFactor(double valueFactor) const { return valueFactor; }
  void setValueFactor(double valueFactor) { this->valueFactor = valueFactor; }

 protected:
  std::unique_ptr<ScalarFunction> fOrig;
  base::DataVector lowerBounds;
  base::DataVector upperBounds;
  double valueFactor;
  base::DataVector xScaled;
};
}  // namespace optimization
}  // namespace sgpp

#endif /* SGPP_OPTIMIZATION_FUNCTION_SCALAR_SCALEDSCALARFUNCTION_HPP */
