// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/combigrid/operation/onedim/PolynomialInterpolationEvaluator.hpp>
#include <limits>
#include <cmath>
#include <vector>

namespace sgpp {
namespace combigrid {

PolynomialInterpolationEvaluator::PolynomialInterpolationEvaluator(
    PolynomialInterpolationEvaluator const &other)
    : evaluationPoint(other.evaluationPoint),
      basisCoefficients(other.basisCoefficients),
      wValues(other.wValues),
      xValues(other.xValues) {}

PolynomialInterpolationEvaluator::PolynomialInterpolationEvaluator()
    : evaluationPoint(0.0), basisCoefficients(), wValues(), xValues() {}

PolynomialInterpolationEvaluator::~PolynomialInterpolationEvaluator() {}

void PolynomialInterpolationEvaluator::setGridPoints(std::vector<double> const &newXValues) {
  this->xValues = newXValues;
  size_t numPoints = xValues.size();
  wValues.resize(numPoints);

  for (size_t i = 0; i < numPoints; ++i) {
    auto x = xValues[i];
    double wInv = 1.0;

    for (size_t j = 0; j < i; ++j) {
      wInv *= x - xValues[j];
    }

    for (size_t j = i + 1; j < numPoints; ++j) {
      wInv *= x - xValues[j];
    }

    wValues[i] = 1.0 / wInv;
  }

  computeBasisCoefficients();
}

std::shared_ptr<AbstractLinearEvaluator<FloatScalarVector> >
PolynomialInterpolationEvaluator::cloneLinear() {
  return std::shared_ptr<AbstractLinearEvaluator<FloatScalarVector> >(
      new PolynomialInterpolationEvaluator(*this));
}

bool PolynomialInterpolationEvaluator::needsOrderedPoints() { return false; }

bool PolynomialInterpolationEvaluator::needsParameter() { return true; }

void PolynomialInterpolationEvaluator::computeBasisCoefficients() {
  double sum = 0.0;
  const double minDeviation =
      std::numeric_limits<double>::min() / std::numeric_limits<double>::epsilon();
  size_t numPoints = xValues.size();
  basisCoefficients.resize(numPoints);

  for (size_t i = 0; i < numPoints; ++i) {
    double diff = evaluationPoint - xValues[i];

    // very near to a interpolation value, must not divide by zero
    if (std::abs(diff) < minDeviation) {
      for (size_t j = 0; j < numPoints; ++j) {
        basisCoefficients[j] = (i == j) ? 1.0 : 0.0;
      }
      return;
    }

    double unweightedTerm = wValues[i] / diff;

    sum += unweightedTerm;
    basisCoefficients[i] = unweightedTerm;
  }

  for (size_t i = 0; i < numPoints; ++i) {
    basisCoefficients[i].value() /= sum;
  }
}

void PolynomialInterpolationEvaluator::setParameter(const FloatScalarVector &param) {
  evaluationPoint = param.value();
  computeBasisCoefficients();
}

} /* namespace combigrid */
} /* namespace sgpp*/