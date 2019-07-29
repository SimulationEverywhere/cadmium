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

/**
 * Test that failing to declare the right EOC specification in a coupled model fails compilation
 */

#include <cadmium/basic_model/pdevs/generator.hpp>
#include <cadmium/modeling/coupling.hpp>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/concept/coupled_model_assert.hpp>

// a generator using floating point messages
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

struct out : public cadmium::out_port<int> {
};
using input_ports=std::tuple<>;
using output_ports=std::tuple<out>;

using submodels = cadmium::modeling::models_tuple<floating_generator>;
using EICs = std::tuple<>;
using EOCs = std::tuple<cadmium::modeling::EOC<floating_generator, floating_generator_defs::out, out>>;
using ICs = std::tuple<>;
template<typename TIME>
using C1=cadmium::modeling::pdevs::coupled_model<TIME, input_ports, output_ports, submodels, EICs, EOCs, ICs>;

int main() {
    cadmium::concept::coupled_model_assert<C1>();
    return 0;
}
