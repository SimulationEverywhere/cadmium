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


#ifndef COUPLED_OF_ATOMIC_MODELS_HPP
#define COUPLED_OF_ATOMIC_MODELS_HPP

/**
 * The code below defines a coupled model where all submodels are atomic models
 * This is used as part of multiple compile tests
 */

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/coupling.hpp>
#include <cadmium/concept/coupled_model_assert.hpp>
#include <tuple>

#include <cadmium/basic_model/pdevs/accumulator.hpp>
#include <cadmium/basic_model/pdevs/generator.hpp>
#include <cadmium/basic_model/pdevs/passive.hpp>

using namespace cadmium;
//ports
struct coupled_ports {
    //custom ports
    struct in_1 : public in_port<float> {
    };
    struct in_2 : public in_port<float> {
    };
    struct out : public out_port<float> {
    };
};

//submodels
//passive
template<typename TIME>
using floating_passive=cadmium::basic_models::pdevs::passive<float, TIME>;
using floating_passive_defs=cadmium::basic_models::pdevs::passive_defs<float>;
//generator
const float init_period = 0.1f;
const float init_output_message = 1.0f;
template<typename TIME>
using floating_generator_base=cadmium::basic_models::pdevs::generator<float, TIME>;
using floating_generator_defs=cadmium::basic_models::pdevs::generator_defs<float>;

template<typename TIME>
struct floating_generator : public floating_generator_base<TIME> {
    float period() const override {
        return init_period;
    }

    float output_message() const override {
        return init_output_message;
    }
};
//accumulator
template<typename TIME>
using floating_accumulator=cadmium::basic_models::pdevs::accumulator<float, TIME>;
using floating_accumulator_defs=cadmium::basic_models::pdevs::accumulator_defs<float>;

//Coupled
using namespace cadmium::modeling;
using input_ports=std::tuple<coupled_ports::in_1, coupled_ports::in_2>;
using output_ports=std::tuple<coupled_ports::out>;

using submodels = models_tuple<floating_generator, floating_accumulator, floating_passive>;
using EICs = std::tuple<
        EIC<coupled_ports::in_1, floating_passive, floating_passive_defs::in>,
        EIC<coupled_ports::in_2, floating_passive, floating_passive_defs::in>
>;
using EOCs = std::tuple<
        EOC<floating_accumulator, floating_accumulator_defs::sum, coupled_ports::out>,
        EOC<floating_generator, floating_generator_defs::out, coupled_ports::out>
>;
using ICs = std::tuple<
        IC<floating_generator, floating_generator_defs::out, floating_accumulator, floating_accumulator_defs::add>,
        IC<floating_accumulator, floating_accumulator_defs::sum, floating_passive, floating_passive_defs::in>
>;

template<typename TIME>
using coupled_of_atomics=cadmium::modeling::pdevs::coupled_model<TIME, input_ports, output_ports, submodels, EICs, EOCs, ICs>;

#endif // COUPLED_OF_ATOMIC_MODELS_HPP
