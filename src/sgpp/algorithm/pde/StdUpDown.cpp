/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#include "algorithm/pde/StdUpDown.hpp"

namespace sg
{

StdUpDown::StdUpDown(GridStorage* storage)
{
	this->storage = storage;
}

StdUpDown::~StdUpDown()
{
}

void StdUpDown::mult(DataVector& alpha, DataVector& result)
{
	DataVector beta(result.getSize());
	result.setAll(0.0);
#ifdef USEOMP
#ifdef USEOMPTHREE
	#pragma omp parallel
	{
#ifndef AIX_XLC
		#pragma omp single nowait
#endif
#ifdef AIX_XLC
		#pragma omp single
#endif
		{
			this->updown_parallel(alpha, beta, storage->dim() - 1);
		}
	}
	result.add(beta);
#endif
#ifndef USEOMPTHREE
	this->updown(alpha, beta, storage->dim() - 1);

	result.add(beta);
#endif
#endif
#ifndef USEOMP
	this->updown(alpha, beta, storage->dim() - 1);

	result.add(beta);
#endif
}

#ifndef USEOMPTHREE
void StdUpDown::updown(DataVector& alpha, DataVector& result, size_t dim)
{
	//Unidirectional scheme
	if(dim > 0)
	{
		// Reordering ups and downs
		DataVector temp(alpha.getSize());
		up(alpha, temp, dim);
		updown(temp, result, dim-1);

		DataVector result_temp(alpha.getSize());
		updown(alpha, temp, dim-1);
		down(temp, result_temp, dim);

		result.add(result_temp);
	}
	else
	{
		// Terminates dimension recursion
		up(alpha, result, dim);

		DataVector temp(alpha.getSize());
		down(alpha, temp, dim);

		result.add(temp);
	}
}
#endif

#ifdef USEOMPTHREE
void StdUpDown::updown_parallel(DataVector& alpha, DataVector& result, size_t dim)
{
	//Unidirectional scheme
	if(dim > 0)
	{
		// Reordering ups and downs
		DataVector temp(alpha.getSize());
		DataVector result_temp(alpha.getSize());
		DataVector temp_two(alpha.getSize());

		#pragma omp task shared(alpha, temp, result)
		{
			up(alpha, temp, dim);
			updown_parallel(temp, result, dim-1);
		}


		#pragma omp task shared(alpha, temp_two, result_temp)
		{
			updown_parallel(alpha, temp_two, dim-1);
			down(temp_two, result_temp, dim);
		}

		#pragma omp taskwait

		result.add(result_temp);
	}
	else
	{
		// Terminates dimension recursion
		DataVector temp(alpha.getSize());

		#pragma omp task shared(alpha, result)
		up(alpha, result, dim);

		#pragma omp task shared(alpha, temp)
		down(alpha, temp, dim);

		#pragma omp taskwait

		result.add(temp);
	}
}
#endif

}
