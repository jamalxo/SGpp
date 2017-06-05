// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef HASHREFINEMENTMULTIPLECLASS_HPP
#define HASHREFINEMENTMULTIPLECLASS_HPP

#include <sgpp/base/grid/generation/hashmap/HashRefinement.hpp>
#include <sgpp/base/grid/Grid.hpp>
#include <sgpp/base/tools/MultipleClassPoint.hpp>

#include <iostream>
#include <tuple>
#include <cmath>
#include <vector>
#include <algorithm>


namespace sgpp {
namespace base {

/**
 * Refinement class for sparse grids
 * Used by the MultipleClassRefinementFuntor
 * Scores all grid points and refines points based on the information
 * given in the vector of MulitpleClassPoint
 */
class HashRefinementMultipleClass : public HashRefinement {
 public:
  /**
   * Constructor.
   *
   * @param grid Grid to be refined. current_grid_index specifies the grid to be refined
   * @param pts Vector of MultipleClassPoints with additional information
   * @param classGrids Vector of grids
   * @param borderSum Sum of the border scores for all points
   * @param borderCnt amount of points scored towards the border
   * @param topPercent range when densities are concidered close
   */
    HashRefinementMultipleClass(Grid& grid,
        std::vector<sgpp::base::MultipleClassPoint>* pts,
        std::vector<Grid*>& classGrids,
        double &borderSum, double &borderCnt, double topPercent);
    virtual ~HashRefinementMultipleClass() {}

 protected:
  void refineGridpoint(GridStorage& storage, size_t refine_index) override;
  void collectRefinablePoints(GridStorage& storage,
        RefinementFunctor& functor,
        AbstractRefinement::refinement_container_type& collection) override;

 private:
    std::vector<sgpp::base::MultipleClassPoint>* points;
    Grid& multigrid;
    std::vector<Grid*>& grids;
    double &borderSum;
    double &borderCnt;
    double topPercent;

    void addGridpoint(GridStorage& storage, GridPoint& point);
};

} /* namespace base */
} /* namespace sgpp */

#endif /* HASHREFINEMENTMULTIPLECLASS_HPP */
