/**
 * Copyright (c) 2013-2016, Laouen Mayal Louan Belloli
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
#include "dynamic_atomic_helpers.hpp"

namespace cadmium {
    namespace modeling {

        using cadmium::dynamic_bag;

        template<template<typename T> class ATOMIC, typename TIME>
        class dynamic_atomic : atomic {

            // wrapped atomic model;
            ATOMIC<TIME> _model;

        public:

            // Required types interface
            using state_type = typename ATOMIC<TIME>::state_type;
            using output_ports = typename ATOMIC<TIME>::output_ports;
            using input_ports = typename ATOMIC<TIME>::input_ports;

            // Model input and output types
            using output_bags = typename make_message_bags<output_ports>::type;
            using input_bags = typename make_message_bags<input_ports>::type;

            // Required members interface
            state_type state;

            dynamic_atomic() {
                static_assert(cadmium::concept::is_atomic<ATOMIC>::value, "This is not an atomic model");
                state = _model.state;
            };

            void internal_transition() {
                _model.internal_transition();
                state = _model.state;
            };

            void external_transition(TIME e, cadmium::dynamic_message_bags dynamic_bags) {

                input_bags bags;
                cadmium::modeling::fill_bags_from_map(dynamic_bags, bags);
                _model.external_transition(e, bags);
                state = _model.state;
            };

            void confluence_transition(TIME e, cadmium::dynamic_message_bags dynamic_bags) {

                input_bags bags;
                cadmium::modeling::fill_bags_from_map(dynamic_bags, bags);
                this->_model.confluence_transition(e, bags);
                state = _model.state;
            };

            cadmium::dynamic_message_bags output() const {

                cadmium::dynamic_message_bags dynamic_bags;
                cadmium::modeling::fill_map_from_bags(_model.output(), dynamic_bags);
                return dynamic_bags;
            };

            TIME time_advance() const {
                return _model.time_advance();
            };
        };
    }
}

#endif // CADMIUM_DYNAMIC_ATOMIC_HPP