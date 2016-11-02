/**
 * Copyright (c) 2013-2016, 
 * Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis
 * Cristina Ruiz Martín
 * Carleton University, Universidad de Valladolid
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
 * Test that asserting coupled model with all submodels atomic does not fail compilation
 */

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/coupled_model.hpp>
#include <cadmium/concept/coupled_model_assert.hpp>
#include <tuple>

#include <cadmium/basic_model/accumulator2.hpp>

using namespace cadmium;

 struct reset_tick {
    };
//ports
struct coupled_ports{
    //custom ports
   

    struct add : public in_port<float> {};
    struct reset : public in_port<reset_tick> {};
    struct out : public out_port<float> {};
};


//submodels
//passive
template<typename TIME>
using floating_accumulator=cadmium::basic_models::accumulator2<float, reset_tick, TIME>;
using floating_accumulator_defs=cadmium::basic_models::accumulator2_defs<float, reset_tick>;

//First level coupleds having an accumulator model
using input_ports=std::tuple<coupled_ports::add, coupled_ports::reset>;
using output_ports=std::tuple<coupled_ports::out>;
using submodels = cadmium::modeling::models_tuple<floating_accumulator>;
using EICs = std::tuple<
                    cadmium::modeling::EIC<coupled_ports::add, floating_accumulator, floating_accumulator_defs::add>,
                    cadmium::modeling::EIC<coupled_ports::reset, floating_accumulator, floating_accumulator_defs::reset>
                    >;
using EOCs = std::tuple<cadmium::modeling::EOC<floating_accumulator, floating_accumulator_defs::sum, coupled_ports::out>>;
using ICs = std::tuple<>;
template<typename TIME>
struct C1 :public cadmium::modeling::coupled_model<TIME, input_ports, output_ports, submodels, EICs, EOCs, ICs>{};


//top model
using namespace cadmium::modeling;
using input_ports_top=std::tuple<coupled_ports::add, coupled_ports::reset>;
using output_ports_top=std::tuple<coupled_ports::out>;

using submodels_top = models_tuple<C1, floating_accumulator>;
using EICs_top = std::tuple<
                        EIC<coupled_ports::reset, C1, coupled_ports::reset>,
                        EIC<coupled_ports::add, floating_accumulator, floating_accumulator_defs::add>,
                        EIC<coupled_ports::reset, floating_accumulator, floating_accumulator_defs::reset>
                       >;
using EOCs_top = std::tuple<
                            EOC<C1, coupled_ports::out, coupled_ports::out>
                            >;
using ICs_top = std::tuple<
                        IC<floating_accumulator, floating_accumulator_defs::sum, C1, coupled_ports::add>
                        >;
template<typename TIME>
using C_top=cadmium::modeling::coupled_model<TIME, input_ports_top, output_ports_top, submodels_top, EICs_top, EOCs_top, ICs_top>;


int main(){
    cadmium::concept::coupled_model_assert<C_top>();
    return 0;
}
