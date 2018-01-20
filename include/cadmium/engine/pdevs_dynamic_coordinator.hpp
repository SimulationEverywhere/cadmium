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

#ifndef CADMIUM_PDEVS_DYNAMIC_COORDINATOR_HPP
#define CADMIUM_PDEVS_DYNAMIC_COORDINATOR_HPP

#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/engine/pdevs_dynamic_simulator.hpp>
#include <cadmium/engine/pdevs_dynamic_engine.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/dynamic_common_loggers.hpp>
#include <cadmium/engine/pdevs_dynamic_engine_helpers.hpp>
#include <cadmium/logger/common_loggers.hpp>

namespace cadmium {
    namespace dynamic {
        namespace engine {

            template<typename TIME, typename LOGGER, typename formatter = typename cadmium::dynamic::logger::coordinator_formatter<TIME>, typename simulator_formatter = typename cadmium::dynamic::logger::simulator_formatter<TIME>>
            class coordinator : public cadmium::dynamic::engine::engine<TIME> {

                //MODEL is assumed valid, the whole model tree is checked at "runner level" to fail fast
                TIME _last; //last transition time
                TIME _next; // next transition scheduled

                std::string _model_id;

                subcoordinators_type<TIME> _subcoordinators;
                external_couplings<TIME> _external_output_couplings;
                external_couplings<TIME> _external_input_couplings;
                internal_couplings<TIME> _internal_coupligns;

            public:

                dynamic::message_bags _inbox;
                dynamic::message_bags _outbox;

                using model_type = typename cadmium::dynamic::modeling::coupled<TIME>;

                /**
                 * @brief A dynamic coordinator must be constructed with the coupled model to coordinate.
                 */
                coordinator() = delete;

                coordinator(std::shared_ptr<model_type> coupled_model)
                        : _model_id(coupled_model->get_id())
                {

                    std::map<std::string, std::shared_ptr<engine<TIME>>> enginges_by_id;

                    for(auto& m : coupled_model->_models) {
                        std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::modeling::coupled<TIME>>(m);
                        std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> m_atomic = std::dynamic_pointer_cast<cadmium::dynamic::modeling::atomic_abstract<TIME>>(m);

                        if (m_coupled == nullptr) {
                            if (m_atomic == nullptr) {
                                throw std::domain_error("Invalid submodel is neither coupled nor atomic");
                            }
                            std::shared_ptr<cadmium::dynamic::engine::engine<TIME>> simulator = std::make_shared<cadmium::dynamic::engine::simulator<TIME, LOGGER, simulator_formatter>>(m_atomic);
                            _subcoordinators.push_back(simulator);
                        } else {
                            if (m_atomic != nullptr) {
                                throw std::domain_error("Invalid submodel is defined as both coupled and atomic");
                            }
                            std::shared_ptr<cadmium::dynamic::engine::engine<TIME>> coordinator = std::make_shared<cadmium::dynamic::engine::coordinator<TIME, LOGGER, formatter, simulator_formatter>>(m_coupled);
                            _subcoordinators.push_back(coordinator);
                        }

                        enginges_by_id.insert(std::make_pair(_subcoordinators.back()->get_model_id(), _subcoordinators.back()));
                    }

                    // Generates structures for direct access to external couplings to not iterate all coordinators each time.
                    for (const auto& eoc : coupled_model->_eoc) {
                        if (enginges_by_id.find(eoc._from) == enginges_by_id.end()) {
                            throw std::domain_error("External output coupling from invalid model");
                        }

                        for (auto& coupling : _external_output_couplings) {
                            if (coupling.first->get_model_id() == eoc._from) {
                                coupling.second.push_back(eoc._link);
                                continue;
                            }
                        }
                        cadmium::dynamic::engine::external_coupling<TIME> new_eoc;
                        new_eoc.first = enginges_by_id.at(eoc._from);
                        new_eoc.second.push_back(eoc._link);
                        _external_output_couplings.push_back(new_eoc);
                    }

                    for (const auto& eic : coupled_model->_eic) {
                        if (enginges_by_id.find(eic._to) == enginges_by_id.end()) {
                            throw std::domain_error("External input coupling to invalid model");
                        }

                        for (auto& coupling : _external_input_couplings) {
                            if (coupling.first->get_model_id() == eic._to) {
                                coupling.second.push_back(eic._link);
                                continue;
                            }
                        }
                        cadmium::dynamic::engine::external_coupling<TIME> new_eic;
                        new_eic.first = enginges_by_id.at(eic._to);
                        new_eic.second.push_back(eic._link);
                        _external_input_couplings.push_back(new_eic);
                    }

                    for (const auto& ic : coupled_model->_ic) {
                        if (enginges_by_id.find(ic._from) == enginges_by_id.end() || enginges_by_id.find(ic._to) == enginges_by_id.end()) {
                            throw std::domain_error("Internal coupling to invalid model");
                        }

                        for (auto& coupling : _internal_coupligns) {
                            if (coupling.first.first->get_model_id() == ic._from && coupling.first.second->get_model_id() == ic._to) {
                                coupling.second.push_back(ic._link);
                                continue;
                            }
                        }
                        cadmium::dynamic::engine::internal_coupling<TIME> new_ic;
                        new_ic.first.first = enginges_by_id.at(ic._from);
                        new_ic.first.second = enginges_by_id.at(ic._to);
                        new_ic.second.push_back(ic._link);
                        _internal_coupligns.push_back(new_ic);
                    }
                }

                /**
                 * @brief init function sets the start time
                 * @param initial_time is the start time
                 */
                void init(TIME initial_time) override {
                    LOGGER::template log<cadmium::logger::logger_info>(formatter::log_info_init, initial_time, _model_id);

                    _last = initial_time;
                    //init all subcoordinators and find next transition time.
                    cadmium::dynamic::engine::init_subcoordinators<TIME>(initial_time, _subcoordinators);
                    //find the one with the lowest next time
                    _next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);
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

                /**
                 * @brief Collects outputs ready for output before advancing the simulation
                 * @param t time the simulation will be advanced to
                 * @todo At this point all messages are copied while routed from one level to the next, need to find a good
                 * strategy to lower copying, maybe.
                 * @todo Merge the Collect output calls into the advance simulation as done with ICs and EICs routing
                 */
                void collect_outputs(const TIME &t) override {
                    LOGGER::template log<cadmium::logger::logger_info>(formatter::log_info_collect, t, _model_id);

                    //collecting if necessary
                    if (_next < t) {
                        throw std::domain_error("Trying to obtain output when not internal event is scheduled");
                    } else if (_next == t) {
                        //log EOC
                        LOGGER::template log<cadmium::logger::logger_message_routing>(formatter::log_routing_collect, t, _model_id);

                        // Fill all outboxes and clean the inboxes in the lower levels recursively
                        cadmium::dynamic::engine::collect_outputs_in_subcoordinators<TIME>(t, _subcoordinators);

                        // Use the EOC mapping to compose current level output
                        _outbox = cadmium::dynamic::engine::collect_messages_by_eoc<TIME, LOGGER>(_external_output_couplings);
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

                /**
                 * @brief advanceSimulation advances the execution to t, at t introduces the messages into the system (if any).
                 * @param t is the time the transition is expected to be run.
                 */
                void advance_simulation(const TIME &t) override {
                    //clean outbox because messages are routed before calling this function at a higher level
                    _outbox = cadmium::dynamic::message_bags();

                    LOGGER::template log<cadmium::logger::logger_info>(formatter::log_info_advance, _last, t, _model_id);

                    if (_next < t || t < _last ) {
                        throw std::domain_error("Trying to obtain output when out of the advance time scope");
                    } else {

                        //Route the messages standing in the outboxes to mapped inboxes following ICs and EICs
                        LOGGER::template log<cadmium::logger::logger_message_routing>(formatter::log_routing_ic_collect, t, _model_id);
                        cadmium::dynamic::engine::route_internal_coupled_messages_on_subcoordinators<TIME, LOGGER>(_internal_coupligns);

                        LOGGER::template log<cadmium::logger::logger_message_routing>(formatter::log_routing_eic_collect, t, _model_id);
                        cadmium::dynamic::engine::route_external_input_coupled_messages_on_subcoordinators<TIME, LOGGER>(_inbox, _external_input_couplings);

                        //recurse on advance_simulation
                        cadmium::dynamic::engine::advance_simulation_in_subengines<TIME>(t, _subcoordinators);

                        //set _last and _next
                        _last = t;
                        _next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);

                        //clean inbox because they were processed already
                        _inbox = cadmium::dynamic::message_bags();
                    }
                }
            };
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_COORDINATOR_HPP
