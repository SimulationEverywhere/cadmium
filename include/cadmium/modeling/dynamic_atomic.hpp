/**
 * Copyright (c) 2017, Laouen M. L. Belloli
 * Carleton University, Universidad de Buenos Aires
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

#ifndef CADMIUM_DYNAMIC_ATOMIC_HPP
#define CADMIUM_DYNAMIC_ATOMIC_HPP

#include <map>
#include <boost/any.hpp>
#include <cadmium/modeling/atomic.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/concept/concept_helpers.hpp>
#include <cadmium/modeling/dynamic_atomic_helpers.hpp>

namespace cadmium {
    namespace modeling {

        /**
         * @brief dynamic_atomic is a derived class from the base classes atomic and ATOMIC<TIME>
         * this allow using any ATOMIC<TIME> valid class with pointers as an atomic (first abase class) pointer.
         *
         * @details
         * Because ATOMIC<TIME> methods arity are template dependent, this wrapper class uses
         * cadmium::dynamic_message_bags as these methods parameter and it forwards a correct
         * translation to the corresponding type in the wrapped ATOMIC<TIME> base class method.
         *
         * @tparam ATOMIC a valid atomic model class
         * @tparam TIME a valid TIME class to use along with the atomic model class as ATOMIC<TIME>
         */
        template<template<typename T> class ATOMIC, typename TIME>
        class dynamic_atomic : public atomic, public ATOMIC<TIME> {
        public:

            using output_ports = typename ATOMIC<TIME>::output_ports;
            using input_ports = typename ATOMIC<TIME>::input_ports;

            // Model input and output types
            using output_bags = typename make_message_bags<output_ports>::type;
            using input_bags = typename make_message_bags<input_ports>::type;

            dynamic_atomic() {
                static_assert(cadmium::concept::is_atomic<ATOMIC>::value, "This is not an atomic model");
            };

            void external_transition(TIME e, cadmium::dynamic_message_bags dynamic_bags) {
                // Translate from dynamic_message_bag to template dependent input_bags type.
                input_bags bags;
                cadmium::modeling::fill_bags_from_map(dynamic_bags, bags);

                // Forwards the translated value to the wrapped ATOMIC<TIME> class method.
                external_transition(e, bags);
            };

            void confluence_transition(TIME e, cadmium::dynamic_message_bags dynamic_bags) {
                // Translate from dynamic_message_bag to template dependent input_bags type.
                input_bags bags;
                cadmium::modeling::fill_bags_from_map(dynamic_bags, bags);

                // Forwards the translated value to the wrapped ATOMIC<TIME> class method.
                confluence_transition(e, bags);
            };

            cadmium::dynamic_message_bags output() const {
                cadmium::dynamic_message_bags dynamic_bags;
                output_bags bags = output();

                // Translate from template dependent output_bags type to dynamic_message_bag.
                cadmium::modeling::fill_map_from_bags(bags, dynamic_bags);
                return dynamic_bags;
            };
        };
    }
}

#endif // CADMIUM_DYNAMIC_ATOMIC_HPP