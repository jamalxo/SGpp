/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/

#ifndef VARTIMESTEP_HPP
#define VARTIMESTEP_HPP

#include "base/application/ScreenOutput.hpp"
#include "base/solver/ODESolver.hpp"
#include <string>
//
namespace sg
{
namespace solver
{

/**
 * This class implements a step size control using Adams-Bashforth and Crank-Nicolson
 * for solving ordinary partial equations
 *
 * @version $HEAD$
 */
class VarTimestep : public ODESolver
{
private:
	/// Pointer to sg::base::ScreenOutput object
	sg::base::ScreenOutput* myScreen;

	/// epsilon for the step size control
	double myEps;


public:
	/**
	 * Std-Constructer
	 *
	 * @param Mode the mode of the euler that should be executed, must be ExEul or ImEul
	 * @param imax number of maximum executed iterations
	 * @param timestepSize the size of one timestep
	 * @param eps the epsilon for the step size control
	 * @param screen possible pointer to a sg::base::ScreenOutput object
	 */
	VarTimestep(size_t imax, double timestepSize, double eps, sg::base::ScreenOutput* screen = NULL);

	/**
	 * Std-Destructor
	 */
	virtual ~VarTimestep();

	virtual void solve(SLESolver& LinearSystemSolver, sg::pde::OperationParabolicPDESolverSystem& System, bool bIdentifyLastStep = false, bool verbose = false);
};

}
}

#endif /* VARTIMESTEP_HPP */
