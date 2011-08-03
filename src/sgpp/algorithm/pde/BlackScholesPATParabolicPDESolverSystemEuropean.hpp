/******************************************************************************
* Copyright (C) 2011 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#ifndef BLACKSCHOLESPATPARABOLICPDESOLVERSYSTEMEUROPEAN_HPP
#define BLACKSCHOLESPATPARABOLICPDESOLVERSYSTEMEUROPEAN_HPP

#include "grid/Grid.hpp"
#include "data/DataVector.hpp"
#include "data/DataMatrix.hpp"
#include "operation/pde/OperationParabolicPDESolverSystemDirichlet.hpp"

namespace sg
{
namespace finance
{

/**
 * This class implements the ParabolicPDESolverSystem for the BlackScholes
 * Equation.
 *
 * Here a European Option with fix Dirichlet boundaries is solved.
 *
 * In addition a principal axis transformation is performed in order
 * to reduce the computational effort.
 */
class BlackScholesPATParabolicPDESolverSystemEuropean : public sg::pde::OperationParabolicPDESolverSystemDirichlet
{
protected:
	/// the Laplace Operation, on boundary grid
	sg::base::OperationMatrix* OpLaplaceBound;
	/// the LTwoDotProduct Operation (Mass Matrix), on boundary grid
	sg::base::OperationMatrix* OpLTwoBound;
	/// the Laplace Operation, on Inner grid
	sg::base::OperationMatrix* OpLaplaceInner;
	/// the LTwoDotProduct Operation (Mass Matrix), on Inner grid
	sg::base::OperationMatrix* OpLTwoInner;
	/// Pointer to the lambda (coefficients of the needed Laplace operator)
	sg::base::DataVector* lambda;
	/// use coarsening between timesteps in order to reduce gridsize
	bool useCoarsen;
	/// adaptive mode during solving Black Scholes Equation: coarsen, refine, coarsenNrefine
	std::string adaptSolveMode;
	/// number of points the are coarsened in each coarsening-step !CURRENTLY UNUSED PARAMETER!
	int numCoarsenPoints;
	/// Threshold used to decide if a grid point should be deleted
	double coarsenThreshold;
	/// Threshold used to decide if a grid point should be refined
	double refineThreshold;
	/// refine mode during solving Black Scholes Equation: classic or maxLevel
	std::string refineMode;
	/// maxLevel max. Level of refinement
	size_t refineMaxLevel;
	/// the algorithmic dimensions used in this system
	std::vector<size_t> BSalgoDims;
	/// store number of executed timesteps
	size_t nExecTimesteps;

	virtual void applyLOperatorInner(sg::base::DataVector& alpha, sg::base::DataVector& result);

	virtual void applyLOperatorComplete(sg::base::DataVector& alpha, sg::base::DataVector& result);

	virtual void applyMassMatrixInner(sg::base::DataVector& alpha, sg::base::DataVector& result);

	virtual void applyMassMatrixComplete(sg::base::DataVector& alpha, sg::base::DataVector& result);

public:
	/**
	 * Std-Constructor
	 *
	 * @param SparseGrid reference to the sparse grid
	 * @param alpha the ansatzfunctions' coefficients
	 * @param lambda reference to the lambdas
	 * @param TimestepSize the size of one timestep used in the ODE Solver
	 * @param OperationMode specifies in which solver this matrix is used, valid values are: ExEul for explicit Euler,
	 *  							ImEul for implicit Euler, CrNic for Crank Nicolson solver
	 * @param bLogTransform indicates that this system belongs to a log-transformed Black Scholes Equation
	 * @param useCoarsen specifies if the grid should be coarsened between timesteps
	 * @param coarsenThreshold Threshold to decide, if a grid point should be deleted
	 * @param adaptSolveMode adaptive mode during solving: coarsen, refine, coarsenNrefine
	 * @param numCoarsenPoints number of point that should be coarsened in one coarsening step !CURRENTLY UNUSED PARAMETER!
	 * @param refineThreshold Threshold to decide, if a grid point should be refined
	 * @param refineMode refineMode during solving Black Scholes Equation: classic or maxLevel
	 * @param refineMaxLevel max. level of refinement during solving
	 */
	BlackScholesPATParabolicPDESolverSystemEuropean(sg::base::Grid& SparseGrid, sg::base::DataVector& alpha, sg::base::DataVector& lambda,
			double TimestepSize, std::string OperationMode = "ExEul",
			bool useCoarsen = false, double coarsenThreshold = 0.0, std::string adaptSolveMode ="none",
			int numCoarsenPoints = -1, double refineThreshold = 0.0, std::string refineMode = "classic", size_t refineMaxLevel = 0);

	/**
	 * Std-Destructor
	 */
	virtual ~BlackScholesPATParabolicPDESolverSystemEuropean();

	virtual void finishTimestep(bool isLastTimestep = false);

	void startTimestep();
};

}
}

#endif /* BLACKSCHOLESPATPARABOLICPDESOLVERSYSTEMEUROPEAN_HPP */