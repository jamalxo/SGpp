// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/finance/basis/linear/noboundary/algorithm_sweep/XdPhiPhiUpBBLinear.hpp>

#include <sgpp/globaldef.hpp>


namespace SGPP {
namespace finance {



XdPhiPhiUpBBLinear::XdPhiPhiUpBBLinear(SGPP::base::GridStorage* storage) :
  storage(storage), boundingBox(storage->getBoundingBox()) {
}

XdPhiPhiUpBBLinear::~XdPhiPhiUpBBLinear() {
}

void XdPhiPhiUpBBLinear::operator()(SGPP::base::DataVector& source,
                                    SGPP::base::DataVector& result, grid_iterator& index, size_t dim) {
  float_t q = boundingBox->getIntervalWidth(dim);
  float_t t = boundingBox->getIntervalOffset(dim);

  bool useBB = false;

  if (q != 1.0 || t != 0.0) {
    useBB = true;
  }

  // get boundary values
  float_t fl = 0.0;
  float_t fr = 0.0;

  if (useBB) {
    recBB(source, result, index, dim, fl, fr, q, t);
  } else {
    rec(source, result, index, dim, fl, fr);
  }
}

void XdPhiPhiUpBBLinear::rec(SGPP::base::DataVector& source,
                             SGPP::base::DataVector& result, grid_iterator& index, size_t dim, float_t& fl,
                             float_t& fr) {
  size_t seq = index.seq();

  fl = fr = 0.0;
  float_t fml = 0.0;
  float_t fmr = 0.0;

  SGPP::base::GridStorage::index_type::level_type current_level;
  SGPP::base::GridStorage::index_type::index_type current_index;

  if (!index.hint()) {
    index.leftChild(dim);

    if (!storage->end(index.seq())) {
      rec(source, result, index, dim, fl, fml);
    }

    index.stepRight(dim);

    if (!storage->end(index.seq())) {
      rec(source, result, index, dim, fmr, fr);
    }

    index.up(dim);
  }

  index.get(dim, current_level, current_index);

  float_t fm = fml + fmr;

  float_t alpha_value = source[seq];

  float_t hhalf = 1.0 / static_cast<float_t>(1 << (current_level + 1));
  float_t i_dbl = static_cast<float_t>(current_index);

  // transposed operations:
  result[seq] = fm;

  fl = (fm / 2.0) + ((alpha_value * ((hhalf * i_dbl) - hhalf)) + fl);
  fr = (fm / 2.0) + ((alpha_value * (((-1.0) * (hhalf * i_dbl)) - hhalf)) + fr);
}

void XdPhiPhiUpBBLinear::recBB(SGPP::base::DataVector& source,
                               SGPP::base::DataVector& result, grid_iterator& index, size_t dim, float_t& fl,
                               float_t& fr, float_t q, float_t t) {
  size_t seq = index.seq();

  fl = fr = 0.0;
  float_t fml = 0.0;
  float_t fmr = 0.0;

  SGPP::base::GridStorage::index_type::level_type current_level;
  SGPP::base::GridStorage::index_type::index_type current_index;

  if (!index.hint()) {
    index.leftChild(dim);

    if (!storage->end(index.seq())) {
      recBB(source, result, index, dim, fl, fml, q, t);
    }

    index.stepRight(dim);

    if (!storage->end(index.seq())) {
      recBB(source, result, index, dim, fmr, fr, q, t);
    }

    index.up(dim);
  }

  index.get(dim, current_level, current_index);

  float_t fm = fml + fmr;

  float_t alpha_value = source[seq];

  float_t hhalf = 1.0 / static_cast<float_t>(1 << (current_level + 1));
  float_t i_dbl = static_cast<float_t>(current_index);

  // transposed operations:
  result[seq] = fm;

  fl = (fm / 2.0) + ((alpha_value * ((q * ((hhalf * i_dbl) - hhalf)) +
                                     (0.5 * t))) + fl);
  fr = (fm / 2.0) + ((alpha_value * ((q * (((-1.0) * (hhalf * i_dbl)) - hhalf))
                                     - (0.5 * t))) + fr);
}

// namespace detail

} // namespace SGPP
}
