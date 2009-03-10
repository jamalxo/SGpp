/*****************************************************************************/
/* This file is part of sgpp, a program package making use of spatially      */
/* adaptive sparse grids to solve numerical problems                         */
/*                                                                           */
/* Copyright (C) 2007 Jörg Blank (blankj@in.tum.de)                          */
/* Copyright (C) 2009 Alexander Heinecke (Alexander.Heinecke@mytum.de)       */
/*                                                                           */
/* sgpp is free software; you can redistribute it and/or modify              */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 3 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* sgpp is distributed in the hope that it will be useful,                   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU Lesser General Public License for more details.                       */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with sgpp; if not, write to the Free Software                       */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/* or see <http://www.gnu.org/licenses/>.                                    */
/*****************************************************************************/

#ifndef LAPLACEDOWNLINEAR_HPP
#define LAPLACEDOWNLINEAR_HPP

#include "grid/GridStorage.hpp"
#include "data/DataVector.h"

namespace sg
{

namespace detail
{

/**
 * down-operation in dimension dim. for use with sweep
 */
class LaplaceDownLinear
{
protected:
	typedef GridStorage::grid_iterator grid_iterator;
	GridStorage* storage;

public:
	LaplaceDownLinear(GridStorage* storage) : storage(storage)
	{
	}

	~LaplaceDownLinear()
	{
	}

	void operator()(DataVector& source, DataVector& result, grid_iterator& index, size_t dim)
	{
		rec(source, result, index, dim, 0.0, 0.0);
	}

protected:

	void rec(DataVector& source, DataVector& result, grid_iterator& index, size_t dim, double fl, double fr)
	{
		size_t seq = index.seq();

		double alpha_value = source[seq];

		{
			GridStorage::index_type::level_type l;
			GridStorage::index_type::index_type i;

			index.get(dim, l, i);

			double h = 1/pow(2.0, l);

			// integration
			result[seq] = (  h * (fl+fr)/2.0
			                      + 2.0/3.0 * h * alpha_value );    // diagonal entry
		}

		// dehierarchisation
		double fm = (fl+fr)/2.0 + alpha_value;

		if(!index.hint(dim))
		{
			index.left_child(dim);
			if(!storage->end(index.seq()))
			{
				rec(source, result, index, dim, fl, fm);
			}

			index.step_right(dim);
			if(!storage->end(index.seq()))
			{
				rec(source, result, index, dim, fm, fr);
			}

			index.up(dim);
		}
	}


};

} // namespace detail

} // namespace sg

#endif /* LAPLACEDOWNLINEAR_HPP */
