/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#ifndef PHIPHIDOWNBBLINEARBOUNDARY_HPP
#define PHIPHIDOWNBBLINEARBOUNDARY_HPP

#include "grid/GridStorage.hpp"
#include "data/DataVector.hpp"

#include "basis/linear/noboundary/algorithm_sweep/PhiPhiDownBBLinear.hpp"

namespace sg
{

namespace detail
{

/**
 * down-operation in dimension dim. for use with sweep
 */
class PhiPhiDownBBLinearBoundary : public PhiPhiDownBBLinear
{
public:
	/**
	 * Constructor
	 *
	 * @param storage the grid's GridStorage object
	 */
	PhiPhiDownBBLinearBoundary(GridStorage* storage) : PhiPhiDownBBLinear(storage)
	{
	}

	/**
	 * Destructor
	 */
	virtual ~PhiPhiDownBBLinearBoundary()
	{
	}

	/**
	 * This operations performs the calculation of down in the direction of dimension <i>dim</i>
	 *
	 * For level zero it's assumed, that both ansatz-functions do exist: 0,0 and 0,1
	 * If one is missing this code might produce some bad errors (segmentation fault, wrong calculation
	 * result)
	 * So please assure that both functions do exist!
	 *
	 * On level zero the getfixDirechletBoundaries of the storage object evaluated
	 *
	 * @param source DataVector that contains the gridpoint's coefficients (values from the vector of the laplace operation)
	 * @param result DataVector that contains the result of the down operation
	 * @param index a iterator object of the grid
	 * @param dim current fixed dimension of the 'execution direction'
	 */
	virtual void operator()(DataVector& source, DataVector& result, grid_iterator& index, size_t dim)
	{
		this->q = this->boundingBox->getIntervalWidth(dim);
		this->t = this->boundingBox->getIntervalOffset(dim);

		bool useBB = false;

		if (this->q != 1.0 || this->t != 0.0)
		{
			useBB = true;
		}

		// get boundary values
		double left_boundary;
		double right_boundary;
		size_t seq_left;
		size_t seq_right;

		/*
		 * Handle Level 0
		 */
		// This handles the diagonal only
		//////////////////////////////////////
		// left boundary
		index.left_levelzero(dim);
		seq_left = index.seq();
		left_boundary = source[seq_left];

		// right boundary
		index.right_levelzero(dim);
		seq_right = index.seq();
		right_boundary = source[seq_right];

		if (useBB)
		{
			// check boundary conditions
			if (this->boundingBox->hasDirichletBoundaryLeft(dim))
			{
				result[seq_left] = 0.0; //left_boundary
			}
			else
			{
				result[seq_left] = ((1.0/3.0)*left_boundary)*this->q;
			}
			if (this->boundingBox->hasDirichletBoundaryRight(dim))
			{
				result[seq_right] = 0.0; //right_boundary;
			}
			else
			{
				result[seq_right] = ((1.0/3.0)*right_boundary)*this->q;

				// down
				//////////////////////////////////////
				result[seq_right] += ((1.0/6.0)*left_boundary)*this->q;
			}

			// move to root
			if (!index.hint())
			{
				index.top(dim);

				if(!this->storage->end(index.seq()))
				{
					recBB(source, result, index, dim, left_boundary, right_boundary);
				}

				index.left_levelzero(dim);
			}
		}
		else
		{
			// check boundary conditions
			if (this->boundingBox->hasDirichletBoundaryLeft(dim))
			{
				result[seq_left] = 0.0; //left_boundary
			}
			else
			{
				result[seq_left] = (1.0/3.0)*left_boundary;
			}
			if (this->boundingBox->hasDirichletBoundaryRight(dim))
			{
				result[seq_right] = 0.0; //right_boundary;
			}
			else
			{
				result[seq_right] = (1.0/3.0)*right_boundary;

				// down
				//////////////////////////////////////
				result[seq_right] += (1.0/6.0)*left_boundary;
			}

			// move to root
			if (!index.hint())
			{
				index.top(dim);

				if(!this->storage->end(index.seq()))
				{
					rec(source, result, index, dim, left_boundary, right_boundary);
				}

				index.left_levelzero(dim);
			}
		}
	}
};

} // namespace detail

} // namespace sg

#endif /* PHIPHIDOWNBBLINEARBOUNDARY_HPP */
