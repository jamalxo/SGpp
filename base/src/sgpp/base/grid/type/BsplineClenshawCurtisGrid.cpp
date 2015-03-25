// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/base/grid/Grid.hpp>
#include <sgpp/base/grid/type/BsplineClenshawCurtisGrid.hpp>

#include <sgpp/base/grid/generation/TruncatedBoundaryGridGenerator.hpp>

#include <sgpp/base/exception/factory_exception.hpp>


#include <iostream>

#include <sgpp/globaldef.hpp>


namespace SGPP {
  namespace base {

    BsplineClenshawCurtisGrid::BsplineClenshawCurtisGrid(std::istream& istr) : Grid(istr), degree(1 << 16), basis_(NULL) {
      istr >> degree;
    }


    BsplineClenshawCurtisGrid::BsplineClenshawCurtisGrid(size_t dim, size_t degree) : degree(degree), basis_(NULL) {
      this->storage = new GridStorage(dim);
    }

    BsplineClenshawCurtisGrid::~BsplineClenshawCurtisGrid() {
      if (basis_ != NULL) {
        delete basis_;
      }
    }

    const char* BsplineClenshawCurtisGrid::getType() {
      return "bsplineClenshawCurtis";
    }

    const SBasis& BsplineClenshawCurtisGrid::getBasis() {
      if (basis_ == NULL) {
        basis_ = new SBsplineClenshawCurtisBase(degree);
      }

      return *basis_;
    }

    size_t BsplineClenshawCurtisGrid::getDegree() {
      return this->degree;
    }

    Grid* BsplineClenshawCurtisGrid::unserialize(std::istream& istr) {
      return new BsplineClenshawCurtisGrid(istr);
    }

    void BsplineClenshawCurtisGrid::serialize(std::ostream& ostr) {
      this->Grid::serialize(ostr);
      ostr << degree << std::endl;
    }

    /**
     * Creates new GridGenerator
     * This must be changed if we add other storage types
     */
    GridGenerator* BsplineClenshawCurtisGrid::createGridGenerator() {
      return new TruncatedBoundaryGridGenerator(this->storage);
    }

  }
}