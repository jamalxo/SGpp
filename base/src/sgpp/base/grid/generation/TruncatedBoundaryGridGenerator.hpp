// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef TRUNCATEDBOUNDARYGRIDGENERATOR_HPP
#define TRUNCATEDBOUNDARYGRIDGENERATOR_HPP

#include <sgpp/base/grid/GridStorage.hpp>
#include <sgpp/base/grid/generation/GridGenerator.hpp>

#include <sgpp/globaldef.hpp>


namespace SGPP {
  namespace base {

    /**
     * This class provides the interface for the grid generation
     * for grids with boundaries, pentagon cut through sub space scheme
     */
    class TruncatedBoundaryGridGenerator : public GridGenerator {
      public:
        /**
         * Constructor
         *
         * @param storage template type that holds the grid points
         */
        TruncatedBoundaryGridGenerator(GridStorage* storage);

        /**
         * Destructor
         */
        virtual ~TruncatedBoundaryGridGenerator();

        virtual void regular(size_t level);
        virtual void cliques(size_t level, size_t clique_size);
        virtual void full(size_t level);
        virtual void refine(RefinementFunctor* func);
        virtual size_t getNumberOfRefinablePoints();

        virtual void coarsen(CoarseningFunctor* func, DataVector* alpha);
        virtual void coarsenNFirstOnly(CoarseningFunctor* func, DataVector* alpha, size_t numFirstOnly);
        virtual size_t getNumberOfRemovablePoints();

        virtual void refineMaxLevel(RefinementFunctor* func, size_t maxLevel);
        virtual size_t getNumberOfRefinablePointsToMaxLevel(size_t maxLevel);

      protected:
        /// Pointer to the grid's storage object
        GridStorage* storage;
    };

  }
}

#endif /* TRUNCATEDBOUNDARYGRIDGENERATOR_HPP */
