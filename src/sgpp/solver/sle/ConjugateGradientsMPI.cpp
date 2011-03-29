/******************************************************************************
* Copyright (C) 2011 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#ifdef USE_MPI
#include "tools/MPI/SGppMPITools.hpp"
#endif

#include "solver/sle/ConjugateGradientsMPI.hpp"
using namespace sg::solver;
using namespace sg::base;

namespace sg
{
namespace parallel
{

ConjugateGradientsMPI::ConjugateGradientsMPI(size_t imax, double epsilon) : SLESolver(imax, epsilon)
{
}

ConjugateGradientsMPI::~ConjugateGradientsMPI()
{
}

void ConjugateGradientsMPI::solve(OperationMatrix& SystemMatrix, DataVector& alpha, DataVector& b, bool reuse, bool verbose, double max_threshold)
{
	if (myGlobalMPIComm->getMyRank() != 0)
	{
		this->waitForTask(SystemMatrix, alpha);
	}
	else
	{
		if (verbose == true)
		{
			std::cout << "Starting Conjugated Gradients" << std::endl;
		}

		// needed for residuum calculation
		double epsilonSquared = this->myEpsilon*this->myEpsilon;
		// number off current iterations
		this->nIterations = 0;

		// define temporal vectors
		DataVector temp(alpha.getSize());
		DataVector q(alpha.getSize());
		DataVector r(b);

		double delta_0 = 0.0;
		double delta_old = 0.0;
		double delta_new = 0.0;
		double beta = 0.0;
		double a = 0.0;

		if (verbose == true)
		{
			std::cout << "All temp variables used in CG have been initialized" << std::endl;
		}

		if (reuse == true)
		{
			q.setAll(0.0);

			myGlobalMPIComm->broadcastControl('M');
			SystemMatrix.mult(q, temp);

			r.sub(temp);
			delta_0 = r.dotProduct(r)*epsilonSquared;
		}
		else
		{
			alpha.setAll(0.0);
		}

		// calculate the starting residuum
		myGlobalMPIComm->broadcastControl('M');
		SystemMatrix.mult(alpha, temp);

		r.sub(temp);

		DataVector d(r);

		delta_old = 0.0;
		delta_new = r.dotProduct(r);

		if (reuse == false)
		{
			delta_0 = delta_new*epsilonSquared;
		}

		this->residuum = (delta_0/epsilonSquared);

		if (verbose == true)
		{
			std::cout << "Starting norm of residuum: " << (delta_0/epsilonSquared) << std::endl;
			std::cout << "Target norm:               " << (delta_0) << std::endl;
		}

		while ((this->nIterations < this->nMaxIterations) && (delta_new > delta_0) && (delta_new > max_threshold))
		{
			// q = A*d
			myGlobalMPIComm->broadcastControl('M');
			SystemMatrix.mult(d, q);

			// a = d_new / d.q
			a = delta_new/d.dotProduct(q);

			// x = x + a*d
			alpha.axpy(a, d);

			// Why ????
			if ((this->nIterations % 50) == 0)
			{
				// r = b - A*x
				myGlobalMPIComm->broadcastControl('M');
				SystemMatrix.mult(alpha, temp);

				r.copyFrom(b);
				r.sub(temp);
			}
			else
			{
				// r = r - a*q
				r.axpy(-a, q);
			}


			// calculate new deltas and determine beta
			delta_old = delta_new;
			delta_new = r.dotProduct(r);
			beta = delta_new/delta_old;

			this->residuum = delta_new;

			if (verbose == true)
			{
				std::cout << "delta: " << delta_new << std::endl;
			}

			d.mult(beta);
			d.add(r);

			this->nIterations++;
		}

		this->residuum = delta_new;

		myGlobalMPIComm->broadcastControl('T');

		if (verbose == true)
		{
			std::cout << "Number of iterations: " << this->nIterations << " (max. " << this->nMaxIterations << ")" << std::endl;
			std::cout << "Final norm of residuum: " << delta_new << std::endl;
		}
	}
}

void ConjugateGradientsMPI::waitForTask(OperationMatrix& SystemMatrix, DataVector& alpha)
{
	char ctrl;
	DataVector result(alpha.getSize());

	do
	{
		ctrl = myGlobalMPIComm->receiveControl();

		if (ctrl == 'M')
		{
			SystemMatrix.mult(alpha, result);
		}
	} while (ctrl != 'T');
}

}
}
