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

#ifndef PDEVS_SIMULATOR_HPP
#define PDEVS_SIMULATOR_HPP
#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/concept/atomic_model_assert.hpp>

/**
 * Simulator implementation for atomic models
 */
namespace cadmium {
    namespace engine {
        template<template<typename T> class MODEL, typename TIME, typename DEBUG=std::false_type>
        //TODO: implement the debugging
        class simulator {
            MODEL<TIME> _model;
            TIME _last;
            TIME _next;
            using input_ports=typename MODEL<TIME>::input_ports;
            using input_bags=typename make_message_bags<input_ports>::type;
            using output_ports=typename MODEL<TIME>::output_ports;
            using output_bags=typename make_message_bags<output_ports>::type;
        public:
            using model_type=MODEL<TIME>;

            /**
             * @brief constructor is used as init function, sets the start time
             * @param t is the start time
             * @return the time until first time advance result
             */
            simulator(TIME initial_time) : _last(initial_time) {
                cadmium::concept::atomic_model_assert<MODEL>();
                _next = initial_time + _model.time_advance();
            }

            output_bags collect_outputs(const TIME &t) {
                if (_next != t)
                    throw std::domain_error("Trying to obtain output when not internal event is scheduled");
                else
                    return _model.output();
            }

            TIME next() {
                return _next;
            }

            void advance_simulation(TIME t, input_bags input) {
                if (!(t >= _last))
                    throw std::domain_error("Event received for executing in the past of current simulation time");
                else if (!(t <= _next))
                    throw std::domain_error("Event received for executing after next internal event");
                else if (all_bags_empty(input)) { //iterate ports, if all empty...
                    if (t != _next)
                        throw std::domain_error("Trying to execute internal transition at wrong time");
                    else
                        _model.internal_transition();
                } else {
                    if (t == _next) { //confluence
                        _model.confluence_transition(t - _last, input);
                    } else { //external
                        _model.external_transition(t - _last, input);
                    }
                }
                _last = t;
                _next = _last + _model.time_advance();
            }
    //TODO: use enable_if functions to give access to read state and messages in debug mode

        private:
            //auxiliary
            template<size_t I, typename... Ps>
            struct all_bags_empty_impl {
                static bool check(std::tuple<Ps...> t) {
                    if (!std::get<I - 1>(t).messages.empty()) return false;
                    return all_bags_empty_impl<I - 1, Ps...>::check(t);
                }
            };

            template<typename... Ps>
            struct all_bags_empty_impl<0, Ps...> {
                static bool check(std::tuple<Ps...> t) {
                    return true;
                }
            };

            template<typename... Ps>
            bool all_bags_empty(std::tuple<Ps...> t) {
                return (std::tuple_size < std::tuple < Ps...>>() == 0 )
                || all_bags_empty_impl<std::tuple_size<decltype(t)>::value, Ps...>::check(t);
            }
        };

    }

}

#endif // PDEVS_SIMULATOR_HPP

