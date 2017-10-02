/**
 * Copyright (c) 2013-2016, Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis
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

#ifndef CADMIUM_PDEVS_SIMULATOR_HPP
#define CADMIUM_PDEVS_SIMULATOR_HPP
#include <sstream>
#include <boost/type_index.hpp>

#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/concept/atomic_model_assert.hpp>
#include <cadmium/engine/pdevs_engine_helpers.hpp>
#include <cadmium/logger/common_loggers.hpp>


/**
 * Simulator implementation for atomic models
 */
namespace cadmium {
    namespace engine {
        template<template<typename T> class MODEL, typename TIME, typename LOGGER>
        //TODO: implement the debugging
        class simulator {
            using input_ports=typename MODEL<TIME>::input_ports;
            using input_bags=typename make_message_bags<input_ports>::type;
            using output_ports=typename MODEL<TIME>::output_ports;
            using in_bags_type=typename make_message_bags<input_ports>::type;
            using out_bags_type=typename make_message_bags<output_ports>::type;
            MODEL<TIME> _model;
            TIME _last;
            TIME _next;

        public://making boxes temporarily public
            //TODO: set boxes back to private
            in_bags_type _inbox;
            out_bags_type _outbox;

        public:
            using model_type=MODEL<TIME>;

            /**
             * @brief simulator constructs by default
             *
             * TODO: create a single parameter constructor receiving init time and remove the init function
             * It will require to call constructors from constructor in coordinator.
             */
//            simulator(){}

            /**
             * @brief constructor is used as init function, sets the start time
             * @param initial_time is the start time
             */
            void init(TIME initial_time) {
                auto log_info_init = [](TIME t) -> std::string {
                     std::ostringstream oss;
                     oss << "Simulator for model ";
                     oss << boost::typeindex::type_id<MODEL<TIME>>().pretty_name();
                     oss << " initialized to time ";
                     oss << t;
                     return oss.str();
                 };
                LOGGER::template log<cadmium::logger::logger_info, decltype(log_info_init), TIME>(log_info_init, initial_time);


                _last=initial_time;
                cadmium::concept::atomic_model_assert<MODEL>();
                _next = initial_time + _model.time_advance();
                
                
                auto log_state = [](const typename model_type::state_type& s) -> std::string {
                    std::ostringstream oss;
                    oss << "State for model ";
                    oss << boost::typeindex::type_id<model_type>().pretty_name();
                    oss << " is ";
                    oss << s;
                    return oss.str();
                };
                
                LOGGER::template log<cadmium::logger::logger_state, decltype(log_state), const typename model_type::state_type&>(log_state, _model.state);

            }


            TIME next() const noexcept{
                return _next;
            }

            void collect_outputs(const TIME &t) {
                auto log_info_collect = [](TIME t) -> std::string {
                     std::ostringstream oss;
                     oss << "Simulator for model ";
                     oss << boost::typeindex::type_id<model_type>().pretty_name();
                     oss << " collecting output at time ";
                     oss << t;
                     return oss.str();
                };
                LOGGER::template log<cadmium::logger::logger_info, decltype(log_info_collect), TIME>(log_info_collect, t);

                //cleanning the inbox and producing outbox
                _inbox = in_bags_type{};

                
                if (_next < t){
                    throw std::domain_error("Trying to obtain output when not internal event is scheduled");
                } else if (_next == t) {
                    _outbox = _model.output();
                } else {
                    _outbox = out_bags_type();
                }

                auto log_messages_collect = [](const out_bags_type& ob) -> std::string {
                     std::ostringstream oss;
                     print_messages_by_port(oss, ob);
                     oss << " generated by model ";
                     oss << boost::typeindex::type_id<model_type>().pretty_name();
                     return oss.str();
                };
                LOGGER::template log<cadmium::logger::logger_messages, decltype(log_messages_collect), out_bags_type>(log_messages_collect, _outbox);

            }

            /**
             * @brief outbox keeps the output generated by the last call to collect_outputs
             */
            out_bags_type outbox() const noexcept{
                return _outbox;
            }


            /**
             * @brief inbox keeps the input introduced by upper level coordinator for running next advance_simulation
             */
            void inbox(in_bags_type in) noexcept{
                _inbox=in;
            }

            /**
             * @brief advanceSimulation advances the execution to t, at t introduces the messages into the system (if any).
             * @param t is the time the transition is expected to be run.
            */
            void advance_simulation(TIME t) {
                //clean outbox because messages are routed before calling this funtion at a higher level
                _outbox = out_bags_type{};

                auto log_info_advance = [](const TIME& from, const TIME& to) -> std::string {
                     std::ostringstream oss;
                     oss << "Simulator for model ";
                     oss << boost::typeindex::type_id<model_type>().pretty_name();
                     oss << " advancing simulation from time ";
                     oss << from;
                     oss << " to ";
                     oss << to;
                     return oss.str();
                };
                LOGGER::template log<cadmium::logger::logger_info, decltype(log_info_advance), TIME>(log_info_advance, _last, t);

                auto log_local_time = [](const TIME& from, const TIME& to) -> std::string {
                    std::ostringstream oss;
                    oss << "Elapsed in model ";
                    oss << boost::typeindex::type_id<model_type>().pretty_name();
                    oss << " is ";
                    oss << (to - from);
                    oss << "s";
                    return oss.str();
                };
                LOGGER::template log<cadmium::logger::logger_local_time, decltype(log_local_time), TIME>(log_local_time, _last, t);

                if (t < _last) {
                    throw std::domain_error("Event received for executing in the past of current simulation time");
                } else if (_next < t) {
                    throw std::domain_error("Event received for executing after next internal event");
                } else {
                    if (!cadmium::engine::all_bags_empty(_inbox)) { //input available
                        if (t == _next) { //confluence
                            _model.confluence_transition(t - _last, _inbox);
                        } else { //external
                            _model.external_transition(t - _last, _inbox);
                        }
                        _last = t;
                        _next = _last + _model.time_advance();
                        //clean inbox because they were processed already
                        _inbox = in_bags_type{};
                    } else { //no input available
                        if (t != _next) {
                            //throw std::domain_error("Trying to execute internal transition at wrong time");
                            //for now, we iterate all models in place of using a FEL.
                            //Then, it could reach the case nothing is there.
                            //Just a nop is enough. And no _next or _last should be changed.
                        } else {
                            _model.internal_transition();
                            _last = t;
                            _next = _last + _model.time_advance();
                        }
                    }
                }

                auto log_state = [](const typename model_type::state_type& s) -> std::string {
                    std::ostringstream oss;
                    oss << "State for model ";
                    oss << boost::typeindex::type_id<model_type>().pretty_name();
                    oss << " is ";
                    oss << s;
                    return oss.str();
                };

                LOGGER::template log<cadmium::logger::logger_state, decltype(log_state), const typename model_type::state_type&>(log_state, _model.state);
            }
    //TODO: use enable_if functions to give access to read state and messages in debug mode
        };

    }

}

#endif // CADMIUM_PDEVS_SIMULATOR_HPP

