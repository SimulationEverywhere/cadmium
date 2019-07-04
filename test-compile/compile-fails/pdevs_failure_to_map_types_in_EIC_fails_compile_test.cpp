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

/**
 * Test that failing to declare the right EIC specification in a coupled model fails compilation when asserted
 */
#include <cadmium/basic_model/pdevs/passive.hpp>
#include <cadmium/modeling/coupled_model.hpp>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/concept/coupled_model_assert.hpp>

template<typename TIME>
using passive = cadmium::basic_models::passive<int, TIME>;
using passive_in = cadmium::basic_models::passive_defs<int>::in;

int main(){
    struct in : public cadmium::in_port<float>{};
    using input_ports=std::tuple<in>;
    using output_ports=std::tuple<>;

    using submodels = cadmium::modeling::models_tuple<passive>;
    using EICs = std::tuple<cadmium::modeling::EIC<in, passive, passive_in>>;
    using EOCs = std::tuple<>;
    using ICs = std::tuple<>;
    using C1=cadmium::modeling::coupled_model<input_ports, output_ports, submodels, EICs, EOCs, ICs>;

    cadmium::concept::coupled_model_assert<C1>();
    return 0;
}
