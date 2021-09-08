/**
 * Copyright (c) 2017, Laouen M. L. Belloli, Damian Vicino
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

#ifndef CADMIUM_PDEVS_DYNAMIC_PARALLEL_SIMULATOR_HPP
#define CADMIUM_PDEVS_DYNAMIC_PARALLEL_SIMULATOR_HPP

#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/parallel_engine/pdevs_dynamic_parallel_engine.hpp>
#include <cadmium/logger/dynamic_common_loggers.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include <cadmium/engine/parallel_helpers.hpp>

namespace cadmium {
    namespace dynamic {
        namespace parallel_engine {

            /**
             * @brief This class is a simulator for dynamic_atomic models.
             *
             * @tparam MODEL - The atomic model type.
             * @tparam TIME - The simulation time type.
             * @tparam LOGGER - The logger type used to log simulation information as model states.
             */
            template<typename TIME, typename LOGGER>
            class parallel_simulator : public parallel_engine<TIME> {
                using model_type=typename cadmium::dynamic::modeling::atomic_abstract<TIME>;

                std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> _model;
                TIME _last;
                TIME _next;

            public:

                cadmium::dynamic::message_bags _outbox;
                cadmium::dynamic::message_bags _inbox;

                parallel_simulator() = delete;

                parallel_simulator(std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> model)
                : _model(model) {}

                /**
                 * @brief sets the last and next times according to the initial_time parameter.
                 *
                 * @param initial_time is the start time
                 */
                cadmium::parallel::info_for_logging<TIME> init(TIME initial_time) {
                    cadmium::parallel::info_for_logging<TIME> log;
                    _last = initial_time;
                    _next = initial_time + _model->time_advance();

                    log.time = initial_time;
                    log.model_id = _model->get_id();
                    log.state_as_string = _model->model_state_as_string();

                    return log;
                }

                std::string get_model_id() const override {
                    return _model->get_id();
                }

                TIME next() const noexcept override {
                    return _next;
                }

                /**
                 * @brief collect_outputs leaves in _outbox the output function in imminent simulators.
                 * @param t is the time when the outputs are collected.
                 */
                cadmium::parallel::info_for_logging<TIME> collect_outputs(const TIME &t) {
                    cadmium::parallel::info_for_logging<TIME> log;

                    // Cleaning the inbox and producing outbox
                    _inbox = cadmium::dynamic::message_bags();

                    if (_next < t) {
                	    throw std::domain_error("Trying to obtain output in a higher time than the next scheduled internal event");
                    } else if (_next == t) {
                	    _outbox = _model->output();
                		std::string messages_by_port = _model->messages_by_port_as_string(_outbox);
                		log.imminent= true;
                		log.messages_by_port = messages_by_port;
                    } else {
                        _outbox = cadmium::dynamic::message_bags();
                	    log.imminent= false;
                    }

                    log.time = t;
                    log.model_id = _model->get_id();
                    return log;
                }

                /**
                 * @brief collect_outputs leaves in _outbox the output function in imminent simulators.
                 * @param t is the time when the outputs are collected.
                 */
                void collect_outputs_no_log(const TIME &t) {

                    // Cleaning the inbox and producing outbox
                    _inbox = cadmium::dynamic::message_bags();

                    if (_next < t) {
                	    throw std::domain_error("Trying to obtain output in a higher time than the next scheduled internal event");
                    } else if (_next == t) {
                	    _outbox = _model->output();
                		std::string messages_by_port = _model->messages_by_port_as_string(_outbox);
                    } else {
                        _outbox = cadmium::dynamic::message_bags();
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
                 * @brief Applies the state transition if its imminent or receiver
                 * @param t is the time the transition is expected to be run.
                */
                cadmium::parallel::info_for_logging<TIME> state_transition(const TIME &t) {
                    cadmium::parallel::info_for_logging<TIME> log;
                    //clean outbox because messages are routed before calling this function at a higher level
                    _outbox = cadmium::dynamic::message_bags();

                    log.time = t;
                    log.last = _last;
                    log.model_id = _model->get_id();

                    if (t < _last) {
                        throw std::domain_error("Event received for executing in the past of current simulation time");
                    } else if (_next < t) {
                        throw std::domain_error("Event received for executing after next internal event");
                    } else {
                        if (!_inbox.empty()) { //input available
                            if (t == _next) { //confluence
                                _model->confluence_transition(t - _last, _inbox);
                            } else { //external
                                _model->external_transition(t - _last, _inbox);
                            }
                            _last = t;
                            _next = _last + _model->time_advance();
                            //clean inbox because they were processed already
                            _inbox = cadmium::dynamic::message_bags();
                        } else { //no input available
                            if (t != _next) {
                                //throw std::domain_error("Trying to execute internal transition at wrong time");
                                //for now, we iterate all models in place of using a FEL.
                                //Then, it could reach the case nothing is there.
                                //Just a nop is enough. And no _next or _last should be changed.
                            } else {
                                _model->internal_transition();
                                _last = t;
                                _next = _last + _model->time_advance();
                            }
                        }
                    }
                    log.state_as_string = _model->model_state_as_string();
                    return log;
                }

                /**
                 * @brief Applies the state transition if its imminent or receiver
                 * @param t is the time the transition is expected to be run.
                */
                void state_transition_no_log(const TIME &t) {
                    cadmium::parallel::info_for_logging<TIME> log;
                    //clean outbox because messages are routed before calling this function at a higher level
                    _outbox = cadmium::dynamic::message_bags();

                    if (t < _last) {
                        throw std::domain_error("Event received for executing in the past of current simulation time");
                    } else if (_next < t) {
                        throw std::domain_error("Event received for executing after next internal event");
                    } else {
                        if (!_inbox.empty()) { //input available
                            if (t == _next) { //confluence
                                _model->confluence_transition(t - _last, _inbox);
                            } else { //external
                                _model->external_transition(t - _last, _inbox);
                            }
                            _last = t;
                            _next = _last + _model->time_advance();
                            //clean inbox because they were processed already
                            _inbox = cadmium::dynamic::message_bags();
                        } else { //no input available
                            if (t != _next) {
                                //throw std::domain_error("Trying to execute internal transition at wrong time");
                                //for now, we iterate all models in place of using a FEL.
                                //Then, it could reach the case nothing is there.
                                //Just a nop is enough. And no _next or _last should be changed.
                            } else {
                                _model->internal_transition();
                                _last = t;
                                _next = _last + _model->time_advance();
                            }
                        }
                    }
                }


            };
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_SIMULATOR_HPP
