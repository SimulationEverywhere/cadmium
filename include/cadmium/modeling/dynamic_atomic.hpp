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
#include <cadmium/modeling/model.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/concept/concept_helpers.hpp>
#include <cadmium/concept/atomic_model_assert.hpp>
#include <cadmium/modeling/dynamic_models_helpers.hpp>

namespace cadmium {
    namespace dynamic {
        namespace modeling {

            /**
             * @brief atomic is a derived class from the base classes atomic_bastract and ATOMIC<TIME>
             * this allow using any ATOMIC<TIME> valid class with pointers as an atomic_abstract
             * (first abase class) pointer.
             *
             * @details
             * Because ATOMIC<TIME> methods arity are template dependent, this wrapper class uses
             * cadmium::dynamic::message_bags as these methods parameter and it forwards a correct
             * translation to the corresponding type in the wrapped ATOMIC<TIME> base class method.
             *
             * @tparam ATOMIC a valid atomic model class
             * @tparam TIME a valid TIME class to use along with the atomic model class as ATOMIC<TIME>
             */
            template<template<typename T> class ATOMIC, typename TIME>
            class atomic : public atomic_abstract<TIME>, public ATOMIC<TIME> {
            public:
                using model_type=ATOMIC<TIME>;

                using output_ports = typename model_type::output_ports;
                using input_ports = typename model_type::input_ports;

                // Model input and output types
                using output_bags = typename make_message_bags<output_ports>::type;
                using input_bags = typename make_message_bags<input_ports>::type;

                atomic() {
                    static_assert(cadmium::concept::is_atomic<ATOMIC>::value, "This is not an atomic model");
                    cadmium::concept::atomic_model_assert<ATOMIC>();
                }

                std::string get_id() const {
                    return boost::typeindex::type_id<model_type>().pretty_name();
                }

                // This method must be declared to declare all atomic_abstract virtual methods are defined
                void internal_transition() {
                    model_type::internal_transition();
                }

                void external_transition(TIME e, cadmium::dynamic::message_bags bags) {
                    // Translate from dynamic_message_bag to template dependent input_bags type.
                    input_bags tuple_bags;
                    fill_bags_from_map(bags, tuple_bags);

                    // Forwards the translated value to the wrapped model_type class method.
                    model_type::external_transition(e, tuple_bags);
                }

                void confluence_transition(TIME e, cadmium::dynamic::message_bags bags) {
                    // Translate from dynamic_message_bag to template dependent input_bags type.
                    input_bags tuple_bags;
                    fill_bags_from_map(bags, tuple_bags);

                    // Forwards the translated value to the wrapped model_type class method.
                    model_type::confluence_transition(e, tuple_bags);
                }

                cadmium::dynamic::message_bags output() const {
                    cadmium::dynamic::message_bags bags;
                    output_bags tuple_bags = model_type::output();

                    // Translate from template dependent output_bags type to dynamic_message_bag.
                    fill_map_from_bags(tuple_bags, bags);
                    return bags;
                }

                TIME time_advance() const {
                    return model_type::time_advance();
                }
            };

            // TODO(Lao): move this function to the file dynamic_models_helpers, a forward declaration of class atomic must be done.
            template <template<typename T> class ATOMIC, typename TIME>
            std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> make_atomic_model() {
                std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> spModel = std::make_shared<cadmium::dynamic::modeling::atomic<ATOMIC, TIME>>();
                return spModel;
            }
        }
    }
}

#endif // CADMIUM_DYNAMIC_ATOMIC_HPP
