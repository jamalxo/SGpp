/*
 * HyperParameter.h
 *
 *  Created on: 24.01.2018
 *      Author: Eric
 */

#ifndef DATADRIVEN_SRC_SGPP_DATADRIVEN_DATAMINING_MODULES_HPO_HYPERPARAMETER_H_
#define DATADRIVEN_SRC_SGPP_DATADRIVEN_DATAMINING_MODULES_HPO_HYPERPARAMETER_H_

#include <sgpp/datadriven/datamining/modules/hpo/ConfigurationBit.hpp>

#include <list>

namespace sgpp {
namespace datadriven {

class HyperParameter {
public:
	HyperParameter(std::string& name):name(name){}
	virtual ~HyperParameter();
	void makeConfigBits(std::vector<ConfigurationBit*>& configBits);
	virtual void setHarmonica() = 0;

protected:
  std::vector<std::unique_ptr<ConfigurationBit>> bits;
  int nBits;
  std::string name;
};

} /* namespace datadriven */
} /* namespace sgpp */

#endif /* DATADRIVEN_SRC_SGPP_DATADRIVEN_DATAMINING_MODULES_HPO_HYPERPARAMETER_H_ */
