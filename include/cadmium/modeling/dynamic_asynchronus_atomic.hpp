//THIS NEEDS TO BE REFACTORED
//THERE MUST BE A BETTER WAY

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

#ifndef CADMIUM_DYNAMIC_ASYNCHRONUS_ATOMIC_HPP
#define CADMIUM_DYNAMIC_ASYNCHRONUS_ATOMIC_HPP

#include <map>
#include <boost/any.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/concept/concept_helpers.hpp>
#include <cadmium/concept/atomic_model_assert.hpp>
#include <cadmium/modeling/dynamic_models_helpers.hpp>

namespace cadmium {
    namespace dynamic {
        namespace modeling {

            /**
             * @brief atomic is a derived class from the base classes atomic_bastract and ATOMIC<TIME>
             * this allow using any ATOMIC<TIME> valid class with pointers as an asynchronus_atomic_abstract
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
            template<template<typename T> class ATOMIC, typename TIME, typename... Args>
            class asynchronus_atomic : public asynchronus_atomic_abstract<TIME>, public ATOMIC<TIME> {
                cadmium::dynamic::modeling::Ports _input_ports;
                cadmium::dynamic::modeling::Ports _output_ports;

                std::string _id;
                
            public:
                using model_type=ATOMIC<TIME>;

                using output_ports = typename model_type::output_ports;
                using input_ports = typename model_type::input_ports;

                // Model input and output types
                using output_bags = typename make_message_bags<output_ports>::type;
                using input_bags = typename make_message_bags<input_ports>::type;

                asynchronus_atomic() {
                    #ifndef RT_ARM_MBED
                      static_assert(cadmium::concept::is_atomic<ATOMIC>::value, "This is not an atomic model");
                      cadmium::concept::pdevs::atomic_model_assert<ATOMIC>();
                    #endif
                    _id = boost::typeindex::type_id<model_type>().pretty_name();
                    _input_ports = cadmium::dynamic::modeling::create_dynamic_ports<input_ports>();
                    _output_ports = cadmium::dynamic::modeling::create_dynamic_ports<output_ports>();
                }

                asynchronus_atomic(const std::string& model_id, Args&&... args) : 
                AsyncEventSubject(model_id), 
                ATOMIC<TIME>((AsyncEventSubject *)this, std::forward<Args>(args)...) {
                    
                    #ifndef RT_ARM_MBED
                      static_assert(cadmium::concept::is_atomic<ATOMIC>::value, "This is not an atomic model");
                      cadmium::concept::pdevs::atomic_model_assert<ATOMIC>();
                    #endif
                    _id = model_id;
                    _input_ports = cadmium::dynamic::modeling::create_dynamic_ports<input_ports>();
                    _output_ports = cadmium::dynamic::modeling::create_dynamic_ports<output_ports>();
                }

                std::string get_id() const override {
                    return _id;
                }

                cadmium::dynamic::modeling::Ports get_input_ports() const override {
                    return _input_ports;
                }

                cadmium::dynamic::modeling::Ports get_output_ports() const override {
                    return _output_ports;
                }

                std::string model_state_as_string() const override {
                    std::ostringstream oss;
                    oss << this->state;
                    return oss.str();
                }

                std::string messages_by_port_as_string(cadmium::dynamic::message_bags outbox) const override {
                    std::ostringstream oss;
                    print_dynamic_messages_by_port<output_ports>(oss, outbox);
                    return oss.str();
                }

                // This method must be declared to declare all asynchronus_atomic_abstract virtual methods are defined
                void internal_transition() override {
                    model_type::internal_transition();
                }

                void external_transition(TIME e, cadmium::dynamic::message_bags bags) override {
                    // Translate from dynamic_message_bag to template dependent input_bags type.
                    input_bags tuple_bags;
                    cadmium::dynamic::modeling::fill_bags_from_map(bags, tuple_bags);

                    // Forwards the translated value to the wrapped model_type class method.
                    model_type::external_transition(e, tuple_bags);
                }

                void confluence_transition(TIME e, cadmium::dynamic::message_bags bags) override {
                    // Translate from dynamic_message_bag to template dependent input_bags type.
                    input_bags tuple_bags;
                    cadmium::dynamic::modeling::fill_bags_from_map(bags, tuple_bags);

                    // Forwards the translated value to the wrapped model_type class method.
                    model_type::confluence_transition(e, tuple_bags);
                }

                cadmium::dynamic::message_bags output() const override {
                    cadmium::dynamic::message_bags bags;
                    output_bags tuple_bags = model_type::output();

                    // Translate from template dependent output_bags type to dynamic_message_bag.
                    cadmium::dynamic::modeling::fill_map_from_bags(tuple_bags, bags);
                    return bags;
                }

                TIME time_advance() const override {
                    return model_type::time_advance();
                }
            };
        }
    }
}

#endif // CADMIUM_DYNAMIC_ASYNCHRONUS_ATOMIC_HPP
