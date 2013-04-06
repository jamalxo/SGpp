/* *****************************************************************************
* Copyright (C) 2013 Technische Universitaet Muenchen                          *
* This file is part of the SG++ project. For conditions of distribution and    *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp           *
***************************************************************************** */
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#ifndef OPERATIONLAPLACEENHANCEDLINEARBOUNDARY_HPP
#define OPERATIONLAPLACEENHANCEDLINEARBOUNDARY_HPP

#include "experimental/pde/algorithm/UpDownOneOpDimEnhanced.hpp"

namespace sg
{
namespace pde
{

/**
 * Implements the Laplace operator based on
 * the UpDownOneOpDimEnhanced method.
 *
 * @version $HEAD$
 */
class OperationLaplaceEnhancedLinearBoundary: public UpDownOneOpDimEnhanced
{
public:
	/**
	 * Constructor
	 *
	 * @param storage the grid's sg::base::GridStorage object
	 */
	OperationLaplaceEnhancedLinearBoundary(sg::base::GridStorage* storage);

	/**
	 * Constructor of OperationLaplaceLinear
	 *
	 * @param storage Pointer to the grid's gridstorage obejct
	 * @param coef reference to a sg::base::DataVector object that contains the bilinear form's constant coefficients; one per dimension
	 */
	OperationLaplaceEnhancedLinearBoundary(sg::base::GridStorage* storage, sg::base::DataVector& coef);

	/**
	 * Destructor
	 */
	virtual ~OperationLaplaceEnhancedLinearBoundary();

protected:
	/**
	 * Up-step
	 *
	 * @param dim dimension in which to apply the up-part
	 * @param alpha vector of coefficients
	 * @param result vector to store the results in
	 */
	virtual void up(sg::base::DataMatrix& alpha, sg::base::DataMatrix& result, size_t dim);

	/**
	 * Down-step
	 *
	 * @param dim dimension in which to apply the down-part
	 * @param alpha vector of coefficients
	 * @param result vector to store the results in
	 */
	virtual void down(sg::base::DataMatrix& alpha, sg::base::DataMatrix& result, size_t dim);
};

}
}

#endif /* OPERATIONLAPLACEENHANCEDLINEARBOUNDARY_HPP */
