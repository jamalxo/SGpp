/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#ifndef BLACKSCHOLESTIMESTEPMATRIX_HPP
#define BLACKSCHOLESTIMESTEPMATRIX_HPP

#include "grid/Grid.hpp"
#include "data/DataVector.hpp"
#include "operation/common/OperationODESolverMatrix.hpp"
#include "grid/common/DirichletUpdateVector.hpp"

namespace sg
{

/**
 * @todo (heinecke) add description here
 */
class BlackScholesTimestepMatrix : public OperationODESolverMatrix
{
private:
	/// the riskfree interest rate
	double r;
	/// the delta Operation
	OperationMatrix* OpDelta;
	/// the Gamma Operation
	OperationMatrix* OpGamma;
	/// Pointer to the mus
	DataVector* mus;
	/// Pointer to the sigmas
	DataVector* sigmas;
	/// Pointer to the rhos;
	DataVector* rhos;
	/// Pointer to the coefficients of operation Delta
	DataVector* deltaCoef;
	/// Pointer to the coefficients ot operation Gamma
	DataVector* gammaCoef;
	/// the LTwoDotProduct Operation (Mass Matrix)
	OperationMatrix* OpLTwo;
	/// Routine to modify the boundaries of the grid
	DirichletUpdateVector* BoundaryUpdate;
	/**
	 *  specifies in which solver this matrix is used, valid values are:
	 *  ExEul for explicit Euler
	 *  ImEul for implicit Euler
	 *  CrNic for Crank Nicolson solver
	 */
	std::string tOperationMode;
	// @todo (heinecke) try to do some refactoring here with the timestep size
	/// the size of one timestep used in the ODE Solver
	double TimestepSize;
	/// Pointer to the grid object
	Grid* myGrid;

	/**
	 * Do Matrix mutlitplication with the Black Scholes Systemmatrix
	 *
	 * @param alpha the coefficients of the sparse grid's ansatzfunctions
	 * @param return reference to the DataVector into which the result is written
	 */
	void applyLOperator(DataVector& alpha, DataVector& result);

	/**
	 * Do Matrix mutlitplication with left hand side mass matrix
	 *
	 * @param alpha the coefficients of the sparse grid's ansatzfunctions
	 * @param return reference to the DataVector into which the result is written
	 */
	void applyMassMatrix(DataVector& alpha, DataVector& result);

	/**
	 * Build the coefficients for the Gamma Operation, which
	 * are the assets' covariance matrix multiplied by 0.5
	 *
	 * this routine handles also the symmtrie of the
	 * gamma operation
	 */
	void buildGammaCoefficients();

	/**
	 * Build the coefficients for the combined Delta Operation
	 */
	void buildDeltaCoefficients();

public:
	/**
	 * Std-Constructor
	 *
	 * @param SparseGrid reference to the sparse grid
	 * @param mu reference to the mus
	 * @param sigma reference to the sigmas
	 * @param rho reference to the rhos
	 * @param r the riskfree interest rate
	 * @param TimestepSize the size of one timestep used in the ODE Solver
	 * @param OperationMode specifies in which solver this matrix is used, valid values are: ExEul for explicit Euler,
	 *  							ImEul for implicit Euler, CrNic for Crank Nicolson solver
	 */
	BlackScholesTimestepMatrix(Grid& SparseGrid, DataVector& mu, DataVector& sigma, DataVector& rho, double r, double TimestepSize, std::string OperationMode = "ExEul");

	/**
	 * Std-Destructor
	 */
	virtual ~BlackScholesTimestepMatrix();

	virtual void mult(DataVector& alpha, DataVector& result);

	virtual void generateRHS(DataVector& data, DataVector& rhs);

	virtual void finishTimestep(DataVector& alpha);

	virtual void startTimestep(DataVector& alpha);

	virtual Grid* getGrid();
};

}

#endif /* BLACKSCHOLESTIMESTEPMATRIX_HPP */
