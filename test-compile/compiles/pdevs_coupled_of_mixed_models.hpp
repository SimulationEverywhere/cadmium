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
 * The code below defines two levels coupled model where submodels are atomic and coupled models
 * This is used as part of multiple compile tests
 */


#ifndef COUPLED_OF_MIXED_MODELS_HPP
#define COUPLED_OF_MIXED_MODELS_HPP

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/coupling.hpp>
#include <cadmium/concept/coupled_model_assert.hpp>
#include <tuple>

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

//First level coupleds having a passive model in each
using input_ports=std::tuple<coupled_ports::in_1, coupled_ports::in_2>;
using output_ports=std::tuple<coupled_ports::out>;
using submodels = cadmium::modeling::models_tuple<floating_passive>;
using EICs = std::tuple<cadmium::modeling::EIC<coupled_ports::in_1, floating_passive, floating_passive_defs::in>>;
using EOCs = std::tuple<>;
using ICs = std::tuple<>;
template<typename TIME>
struct C1
        : public cadmium::modeling::pdevs::coupled_model<TIME, input_ports, output_ports, submodels, EICs, EOCs, ICs> {
};
template<typename TIME>
struct C2
        : public cadmium::modeling::pdevs::coupled_model<TIME, input_ports, output_ports, submodels, EICs, EOCs, ICs> {
};
template<typename TIME>
struct C3
        : public cadmium::modeling::pdevs::coupled_model<TIME, input_ports, output_ports, submodels, EICs, EOCs, ICs> {
};

//top model
using namespace cadmium::modeling;
using input_ports_top=std::tuple<coupled_ports::in_1, coupled_ports::in_2>;
using output_ports_top=std::tuple<coupled_ports::out>;

using submodels_top = models_tuple<C1, C2, floating_passive>;
using EICs_top = std::tuple<
        EIC<coupled_ports::in_1, C1, coupled_ports::in_1>,
        EIC<coupled_ports::in_2, C2, coupled_ports::in_1>,
        EIC<coupled_ports::in_2, floating_passive, floating_passive_defs::in>
>;
using EOCs_top = std::tuple<>;
using ICs_top = std::tuple<>;
template<typename TIME>
using coupled_of_mixed_models=cadmium::modeling::pdevs::coupled_model<TIME, input_ports_top, output_ports_top, submodels_top, EICs_top, EOCs_top, ICs_top>;

#endif // COUPLED_OF_MIXED_MODELS_HPP
