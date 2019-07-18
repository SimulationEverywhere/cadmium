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
 * Test that failing to declare valid submodels in a coupled model fails compilation
 */
#include<cadmium/modeling/coupling.hpp>
#include<cadmium/concept/coupled_model_assert.hpp>

using input_ports_c1=std::tuple<>;
using output_ports_c1=std::tuple<>;
using submodels_c1 = cadmium::modeling::models_tuple<>;
using EICs_c1 = std::tuple<>;
using EOCs_c1 = std::tuple<>;
using ICs_c1 = std::tuple<int>; //C1 is not a valid coupled model
template<typename TIME>
using C1=cadmium::modeling::pdevs::coupled_model<TIME, input_ports_c1, output_ports_c1, submodels_c1, EICs_c1, EOCs_c1, ICs_c1>;

//C2 has C1 as submodel
using input_ports=std::tuple<>;
using output_ports=std::tuple<>;
using submodels = cadmium::modeling::models_tuple<C1>;
using EICs = std::tuple<>;
using EOCs = std::tuple<>;
using ICs = std::tuple<>;
template<typename TIME>
using C2=cadmium::modeling::pdevs::coupled_model<TIME, input_ports, output_ports, submodels, EICs, EOCs, ICs>;

int main(){
    cadmium::concept::pdevs::coupled_model_assert<C2>();
    return 0;
}
