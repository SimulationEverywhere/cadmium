/**
 * Copyright (c) 2013-2015, Damian Vicino
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

#ifndef ATOMIC_MODEL_ASSERT_HPP
#define ATOMIC_MODEL_ASSERT_HPP

#include<cadmium/concept/concept_helper_functions.hpp>
#include<cadmium/modeling/message_bag.hpp>

namespace cadmium{
    namespace concept {
        template<template<typename> class MODEL> //check a template argument is required (for time)
        constexpr void atomic_model_assert() {
            //check portset types are defined
            using ip=typename MODEL<float>::input_ports;
            using op=typename MODEL<float>::output_ports;
            using ip_bags=typename make_message_bags<ip>::type;
            using op_bags=typename make_message_bags<op>::type;
            //check port types are unique for in the portset tuples
            static_assert(check_unique_elem_types<ip>::value(), "ambiguous port name in input ports");
            static_assert(check_unique_elem_types<op>::value(), "ambiguous port name in output ports");
            //check state is declared
            static_assert(std::is_same<typename MODEL<float>::state_type,
                                  decltype(MODEL<float>{}.state)>::value,
                          "state is undefined or has the wrong type");

            //check functions are declared
            static_assert(std::is_same<decltype(std::declval<MODEL<float>>().output()),
                                  op_bags>(),
                          "Output function does not exist or does not return the right message bags");

            static_assert(std::is_same<
                                  decltype(std::declval<MODEL<float>>().confluence_transition(0.0, ip_bags{})),
                                  void>(),
                          "Confluence transition function undefined");

            static_assert(std::is_same<
                                  decltype(std::declval<MODEL<float>>().external_transition(0.0, ip_bags{})),
                                  void>(),
                          "External transition function undefined");

            static_assert(std::is_same<
                                  decltype(std::declval<MODEL<float>>().internal_transition()),
                                  void>(),
                          "Internal transition function undefined");

            static_assert(std::is_same<decltype(std::declval<MODEL<float>>().time_advance()),
                                  float>(),
                          "Time advance function does not exist or does not return the right type of time");
        }
    }
}
#endif // ATOMIC_MODEL_ASSERT_HPP