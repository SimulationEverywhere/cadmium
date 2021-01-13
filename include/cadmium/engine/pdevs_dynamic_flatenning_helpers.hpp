/**
 * Copyright (c) 2020, Guillermo G. Trabes
 * Carleton University, Universidad Nacional de San Luis
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CADMIUM_PDEVS_DYNAMIC_FLATENNING_HELPERS_HPP
#define CADMIUM_PDEVS_DYNAMIC_FLATENNING_HELPERS_HPP

#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/engine/pdevs_dynamic_simulator.hpp>
#include <cadmium/engine/pdevs_dynamic_engine.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/dynamic_common_loggers.hpp>
#include <cadmium/engine/pdevs_dynamic_engine_helpers.hpp>

namespace cadmium {
    namespace dynamic {
        namespace flatenning {

            template<typename TIME, typename LOGGER>
            cadmium::dynamic::engine::coordinator<TIME, LOGGER> flatenning(std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> m_coupled) {
            	//empty method for now

            	cadmium::dynamic::engine::coordinator<TIME, LOGGER> coordinator;

            	std::map<std::string, std::shared_ptr<engine<TIME>>> enginges_by_id;

            	                    	for(auto& m : coupled_model->_models) {
            	                    		std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::modeling::coupled<TIME>>(m);
            	                    		std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> m_atomic = std::dynamic_pointer_cast<cadmium::dynamic::modeling::atomic_abstract<TIME>>(m);

            	                    		if (m_coupled == nullptr) {
            	                    			if (m_atomic == nullptr) {
            	                    				throw std::domain_error("Invalid submodel is neither coupled nor atomic");
            	                    			}
            	                    			std::shared_ptr<cadmium::dynamic::engine::engine<TIME>> simulator = std::make_shared<cadmium::dynamic::engine::simulator<TIME, LOGGER>>(m_atomic);
            	                    			coordinator._subcoordinators.push_back(simulator);
            	                    		} else {
            	                    			if (m_atomic != nullptr) {
            	                    				throw std::domain_error("Invalid submodel is defined as both coupled and atomic");
            	                    			}
            	                    			std::shared_ptr<cadmium::dynamic::engine::engine<TIME>> coordinator = std::make_shared<cadmium::dynamic::engine::coordinator<TIME, LOGGER>>(m_coupled);
            	                    			_subcoordinators.push_back(coordinator);
            	                    		}

            	                    		enginges_by_id.insert(std::make_pair(_subcoordinators.back()->get_model_id(), _subcoordinators.back()));
            	                    	}

            	                    	// Generates structures for direct access to external couplings to not iterate all coordinators each time.
            	                    	bool found;
            	                    	for (const auto& eoc : coupled_model->_eoc) {
            	                    		if (enginges_by_id.find(eoc._from) == enginges_by_id.end()) {
            	                    			throw std::domain_error("External output coupling from invalid model");
            	                    		}

            	                    		found = false;
            	                    		for (auto& coupling : _external_output_couplings) {
            	                    			if (coupling.first->get_model_id() == eoc._from) {
            	                    				coupling.second.push_back(eoc._link);
            	                    				found = true;
            	                    				break;
            	                    			}
            	                    		}

            	                    		if (!found) {
            	                    			cadmium::dynamic::engine::external_coupling<TIME> new_eoc;
            	                    			new_eoc.first = enginges_by_id.at(eoc._from);
            	                    			new_eoc.second.push_back(eoc._link);
            	                    			_external_output_couplings.push_back(new_eoc);
            	                    		}
            	                    	}

            	                    	for (const auto& eic : coupled_model->_eic) {
            	                    		if (enginges_by_id.find(eic._to) == enginges_by_id.end()) {
            	                    			throw std::domain_error("External input coupling to invalid model");
            	                    		}

            	                    		found = false;
            	                    		for (auto& coupling : _external_input_couplings) {
            	                    			if (coupling.first->get_model_id() == eic._to) {
            	                    				coupling.second.push_back(eic._link);
            	                    				found = true;
            	                    				break;
            	                    			}
            	                    		}

            	                    		if (!found) {
            	                    			cadmium::dynamic::engine::external_coupling<TIME> new_eic;
            	                    			new_eic.first = enginges_by_id.at(eic._to);
            	                    			new_eic.second.push_back(eic._link);
            	                    			_external_input_couplings.push_back(new_eic);
            	                    		}
            	                    	}

            	                    	for (const auto& ic : coupled_model->_ic) {
            	                    		if (enginges_by_id.find(ic._from) == enginges_by_id.end() || enginges_by_id.find(ic._to) == enginges_by_id.end()) {
            	                    			throw std::domain_error("Internal coupling to invalid model");
            	                    		}

            	                    		found = false;
            	                    		for (auto& coupling : _internal_coupligns) {
            	                    			if (coupling.first.first->get_model_id() == ic._from && coupling.first.second->get_model_id() == ic._to) {
            	                    				coupling.second.push_back(ic._link);
            	                    				found = true;
            	                    				break;
            	                    			}
            	                    		}

            	                    		if(!found) {
            	                    			cadmium::dynamic::engine::internal_coupling<TIME> new_ic;
            	                    			new_ic.first.first = enginges_by_id.at(ic._from);
            	                    			new_ic.first.second = enginges_by_id.at(ic._to);
            	                    			new_ic.second.push_back(ic._link);
            	                    			_internal_coupligns.push_back(new_ic);
            	                    		}
            	                    	}

            	return coordinator;
            }

        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_FLATENNING_HELPERS_HPP
