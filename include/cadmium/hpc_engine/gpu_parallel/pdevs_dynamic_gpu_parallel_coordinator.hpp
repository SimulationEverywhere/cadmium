/**
 * Copyright (c) 2018, Laouen M. L. Belloli, Damian Vicino
 * Carleton University, Universidad de Buenos Aires, Universite de Nice-Sophia Antipolis
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

#ifndef CADMIUM_PDEVS_DYNAMIC_GPU_PARALLEL_COORDINATOR_HPP
#define CADMIUM_PDEVS_DYNAMIC_GPU_PARALLEL_COORDINATOR_HPP

#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/dynamic_common_loggers.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include <cadmium/hpc_engine/gpu_parallel/pdevs_dynamic_gpu_parallel_engine.hpp>
#include <cadmium/hpc_engine/gpu_parallel/pdevs_dynamic_gpu_parallel_simulator.hpp>
#include <cadmium/hpc_engine/gpu_parallel/pdevs_dynamic_gpu_parallel_engine_helpers.hpp>
#include <cadmium/hpc_engine/gpu_parallel/locks.hpp>
//using namespace cadmium::dynamic::engine;

namespace cadmium {
    namespace dynamic {
        namespace hpc_engine {
            namespace gpu_parallel {
                template<typename TIME, typename LOGGER>
                class gpu_parallel_coordinator : public cadmium::dynamic::hpc_engine::gpu_parallel::gpu_parallel_engine<TIME> {

                    //MODEL is assumed valid, the whole model tree is checked at "runner level" to fail fast
                    TIME _last; //last transition time
                    TIME _next; // next transition scheduled

                    std::string _model_id;

                    subcoordinators_type<TIME, LOGGER> _subcoordinators;
                    external_couplings<TIME, LOGGER> _external_output_couplings;
                    external_couplings<TIME, LOGGER> _external_input_couplings;
                    internal_couplings<TIME, LOGGER> _internal_coupligns;

                    public:

                        dynamic::message_bags _inbox;
                        dynamic::message_bags _outbox;

                        using model_type = typename cadmium::dynamic::modeling::coupled<TIME>;

                        /**
                         * @brief A dynamic coordinator must be constructed with the coupled model to coordinate.
                         */
                    gpu_parallel_coordinator() = delete;

                    gpu_parallel_coordinator(std::shared_ptr<model_type> coupled_model)
                        : _model_id(coupled_model->get_id())
                    {
                        std::map<std::string, std::shared_ptr<gpu_parallel_simulator<TIME, LOGGER>>> engines_by_id;

                        for(auto& m : coupled_model->_models) {
                            std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::modeling::coupled<TIME>>(m);
                            std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> m_atomic = std::dynamic_pointer_cast<cadmium::dynamic::modeling::atomic_abstract<TIME>>(m);

                            if (m_coupled == nullptr) {
                                if (m_atomic == nullptr) {
                                    throw std::domain_error("Invalid submodel is neither coupled nor atomic");
                                }
                                std::shared_ptr<cadmium::dynamic::hpc_engine::gpu_parallel::gpu_parallel_simulator<TIME, LOGGER>> simulator = std::make_shared<cadmium::dynamic::hpc_engine::gpu_parallel::gpu_parallel_simulator<TIME, LOGGER>>(m_atomic);
                                _subcoordinators.push_back(simulator);
                            } else {
                                if (m_atomic != nullptr) {
                                    throw std::domain_error("Invalid submodel is defined as both coupled and atomic");
                                }
                                //std::shared_ptr<cadmium::dynamic::parallel_engine::engine<TIME>> coordinator = std::make_shared<cadmium::dynamic::parallel_engine::cpu_parallel_coordinator<TIME, LOGGER>>(m_coupled);
                                //_subcoordinators.push_back(coordinator);
                            }

                            engines_by_id.insert(std::make_pair(_subcoordinators.back()->get_model_id(), _subcoordinators.back()));
                        }

                        // Generates structures for direct access to external couplings to not iterate all coordinators each time.

                        for (const auto& eoc : coupled_model->_eoc) {
                    	    if (engines_by_id.find(eoc._from) == engines_by_id.end()) {
                    		    throw std::domain_error("External output coupling from invalid model");
                    	    }

                    	    cadmium::dynamic::hpc_engine::gpu_parallel::external_coupling<TIME, LOGGER> new_eoc;
                    	    new_eoc.first = engines_by_id.at(eoc._from);
                    	    new_eoc.second.push_back(eoc._link);
                    	    _external_output_couplings.push_back(new_eoc);
                        }

                        for (const auto& eic : coupled_model->_eic) {
                    	    if (engines_by_id.find(eic._to) == engines_by_id.end()) {
                    		    throw std::domain_error("External input coupling to invalid model");
                    	    }

                    	    cadmium::dynamic::hpc_engine::gpu_parallel::external_coupling<TIME, LOGGER> new_eic;
                    	    new_eic.first = engines_by_id.at(eic._to);
                    	    new_eic.second.push_back(eic._link);
                    	    _external_input_couplings.push_back(new_eic);

                        }

                        for (const auto& ic : coupled_model->_ic) {
                    	    if (engines_by_id.find(ic._from) == engines_by_id.end() || engines_by_id.find(ic._to) == engines_by_id.end()) {
                    		    throw std::domain_error("Internal coupling to invalid model");
                            }

                            cadmium::dynamic::hpc_engine::gpu_parallel::internal_coupling<TIME, LOGGER> new_ic;
                            new_ic.first.first = engines_by_id.at(ic._from);
                            new_ic.first.second = engines_by_id.at(ic._to);
                    	    new_ic.second.push_back(ic._link);
                    	    _internal_coupligns.push_back(new_ic);
                        }

                    }

                    template<typename Iterator>
                    void init_subcoordinators(Iterator first, Iterator last, const TIME &t) {
                        for(; first!=last;first++){
                            _subcoordinators.at(first)->init(t);
                        }
                    }

                    std::string get_model_id() const override {
                        return _model_id;
                    }

                    /**
                     * @brief Coordinator expected next internal transition time
                     */
                    TIME next() const noexcept override {
                        return _next;
                    }

                    TIME last() const noexcept {
                	    return _last;
                    }

                    void set_next(TIME t) {
                	    _next = t;
                    }

                    subcoordinators_type<TIME, LOGGER> subcoordinators() {
                	    return _subcoordinators;
                    }

                    external_couplings<TIME, LOGGER> external_output_couplings() {
                	    return _external_output_couplings;
                    }

                    external_couplings<TIME, LOGGER> external_input_couplings() {
                	    return _external_input_couplings;
                    }

                    internal_couplings<TIME, LOGGER> subcoordinators_internal_couplings() {
                	    return _internal_coupligns;
                    }

                    template<typename Iterator>
                    void collect_outputs_in_subcoordinators(Iterator first, Iterator last, const TIME &t) {
                        for(; first!=last;first++){
                            _subcoordinators.at(first)->collect_outputs(t);
                        }
                    }

                    /**
                     * @brief outbox keeps the output generated by the last call to collect_outputs
                     */
                    cadmium::dynamic::message_bags& outbox() override {
                        return _outbox;
                    }

                    /**
                     * @brief allows to access by reference the internal _inbox member
                     *
                     */
                    cadmium::dynamic::message_bags& inbox() override {
                        return _inbox;
                    }

                    template<typename Iterator>
                    void route_internal_couplings_in_subcoordinators(Iterator first, Iterator last) {

                        auto route_messages = [](auto & c)->void {
                            for (const auto& l : c.second) {
                                auto& from_outbox = c.first.first->outbox();
                                auto& to_inbox = c.first.second->inbox();
                                cadmium::dynamic::logger::routed_messages message_to_log = l->route_messages(from_outbox, to_inbox);

                                #if !defined(NO_LOGGER)
                                cadmium::dynamic::hpc_engine::gpu_parallel::set_log_lock();
                                LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_collect>(message_to_log.from_port, message_to_log.to_port, message_to_log.from_messages, message_to_log.to_messages);
                                cadmium::dynamic::hpc_engine::gpu_parallel::release_log_lock();
                                #endif
                            }
                        };

                        for(; first!=last; first++){
                            route_messages(_internal_coupligns.at(first));
                        }
                    }

                    /**
                     * @brief advanceSimulation advances the execution to t, at t introduces the messages into the system (if any).
                     * @param t is the time the transition is expected to be run.
                     */
                    template<typename Iterator>
                    void state_transition_in_subcoordinators(Iterator first, Iterator last, const TIME &t) {
                        for(; first!=last; first++){
                            _subcoordinators.at(first)->state_transition(t);
                        }
                    }

                    /**
                     * @brief advanceSimulation advances the execution to t, at t introduces the messages into the system (if any).
                     * @param t is the time the transition is expected to be run.
                     */
                    template<typename Iterator>
                    TIME next_in_subcoordinators(Iterator first, Iterator last) {
                        TIME min = _subcoordinators.at(first)->next();

                        for(; first!=last; first++){
                            if(_subcoordinators.at(first)->next() < min){
                                min = _subcoordinators.at(first)->next();
                	        }
                        }

                        return min;
                    }
                };
            }
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_COORDINATOR_HPP
