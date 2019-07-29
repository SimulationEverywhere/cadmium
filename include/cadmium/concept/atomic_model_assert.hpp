/**
 * Copyright (c) 2013-2019, Damian Vicino
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

#ifndef CADMIUM_ATOMIC_MODEL_ASSERT_HPP
#define CADMIUM_ATOMIC_MODEL_ASSERT_HPP

#include<cadmium/concept/concept_helpers.hpp>
#include<cadmium/modeling/message_bag.hpp>
#include<cadmium/modeling/message_box.hpp>

namespace cadmium::concept {
    namespace pdevs {
        //PDEVS
        template<typename FLOATING_MODEL>
        //check a template argument is required (for time)
        constexpr void atomic_model_float_time_assert() {
            //check portset types are defined
            using ip=typename FLOATING_MODEL::input_ports;
            using op=typename FLOATING_MODEL::output_ports;
            using ip_bags=typename make_message_bags<ip>::type;
            using op_bags=typename make_message_bags<op>::type;
            //check port types are unique for in the portset tuples
            static_assert(check_unique_elem_types<ip>::value(), "ambiguous port name in input ports");
            static_assert(check_unique_elem_types<op>::value(), "ambiguous port name in output ports");
            //check state is declared
            static_assert(std::is_same<typename FLOATING_MODEL::state_type,
                                  decltype(FLOATING_MODEL{}.state)>::value,
                          "state is undefined or has the wrong type");

            //check functions are declared
            static_assert(std::is_same<decltype(std::declval<FLOATING_MODEL>().output()),
                                  op_bags>(),
                          "Output function does not exist or does not return the right message bags");

            static_assert(std::is_same<
                                  decltype(std::declval<FLOATING_MODEL>().confluence_transition(0.0, ip_bags{})),
                                  void>(),
                          "Confluence transition function undefined");

            static_assert(std::is_same<
                                  decltype(std::declval<FLOATING_MODEL>().external_transition(0.0, ip_bags{})),
                                  void>(),
                          "External transition function undefined");

            static_assert(std::is_same<
                                  decltype(std::declval<FLOATING_MODEL>().internal_transition()),
                                  void>(),
                          "Internal transition function undefined");

            static_assert(std::is_same<decltype(std::declval<FLOATING_MODEL>().time_advance()),
                                  float>(),
                          "Time advance function does not exist or does not return the right type of time");
        }

        template<template<typename> class MODEL>
        //check a template argument is required (for time)
        constexpr void atomic_model_assert() {
            //setting float as time to use by model
            using floating_model=MODEL<float>;
            atomic_model_float_time_assert<floating_model>();
        }
    }

    //DEVS
    namespace devs {
        template<typename FLOATING_MODEL>
        //check a template argument is required (for time)
        constexpr void atomic_model_float_time_assert() {
            //check portset types are defined
            using ip=typename FLOATING_MODEL::input_ports;
            using op=typename FLOATING_MODEL::output_ports;
            using ip_box=typename make_message_box<ip>::type;
            using op_box=typename make_message_box<op>::type;
            //check port types are unique for in the portset tuples
            static_assert(check_unique_elem_types<ip>::value(), "ambiguous port name in input ports");
            static_assert(check_unique_elem_types<op>::value(), "ambiguous port name in output ports");
            //check state is declared
            static_assert(std::is_same<typename FLOATING_MODEL::state_type,
                                  decltype(FLOATING_MODEL{}.state)>::value,
                          "state is undefined or has the wrong type");

            //check functions are declared
            static_assert(std::is_same<decltype(std::declval<FLOATING_MODEL>().output()),
                                  op_box>(),
                          "Output function does not exist or does not return the right message bags");

            static_assert(std::is_same<
                                  decltype(std::declval<FLOATING_MODEL>().external_transition(0.0, ip_box{})),
                                  void>(),
                          "External transition function undefined");

            static_assert(std::is_same<
                                  decltype(std::declval<FLOATING_MODEL>().internal_transition()),
                                  void>(),
                          "Internal transition function undefined");

            static_assert(std::is_same<decltype(std::declval<FLOATING_MODEL>().time_advance()),
                                  float>(),
                          "Time advance function does not exist or does not return the right type of time");
        }

        template<template<typename> class MODEL>
        //check a template argument is required (for time)
        constexpr void atomic_model_assert() {
            //setting float as time to use by model
            using floating_model=MODEL<float>;
            atomic_model_float_time_assert<floating_model>();
        }
    }
}

#endif // CADMIUM_ATOMIC_MODEL_ASSERT_HPP
