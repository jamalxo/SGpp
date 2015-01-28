#ifndef CLASSIFICATIONREFINEMENTFUNCTOR_HPP
#define CLASSIFICATIONREFINEMENTFUNCTOR_HPP

#include <iostream>
#include "sgpp_base.hpp"

/*
 * Indicator value = number of misclassified data points in the support of a basis function
 */
namespace sg {
  namespace base {

    class ClassificationRefinementFunctor : public RefinementFunctor {
      public:
        ClassificationRefinementFunctor(DataVector* alpha, Grid* grid, size_t refinements_num = 1, double threshold = 0.0);

        virtual ~ClassificationRefinementFunctor();

        virtual double operator()(GridStorage* storage, size_t seq);

        virtual double start();

        size_t getRefinementsNum();

        double getRefinementThreshold();

        void setTrainDataset(DataMatrix* trainDataset);

        void setClasses(DataVector* classes);

      protected:
        /// pointer to the vector that stores the alpha values
        DataVector* alpha;

        /// number of grid points to refine
        size_t refinements_num;

        /// threshold, only the points with greater to equal absolute values of the refinement criterion (e.g. alpha or error) will be refined
        double threshold;

        Grid* grid;
        DataMatrix* trainDataset;
        DataVector* classes;
    };

  }
}

#endif /* CLASSIFICATIONREFINEMENTFUNCTOR_HPP */