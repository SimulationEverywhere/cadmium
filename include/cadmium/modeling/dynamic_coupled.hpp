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

#ifndef CADMIUM_DYNAMIC_COUPLED_HPP
#define CADMIUM_DYNAMIC_COUPLED_HPP

#include <vector>
#include <cassert>
#include <cadmium/modeling/model.hpp>
#include <cadmium/modeling/dynamic_models_helpers.hpp>

namespace cadmium {
    namespace modeling {


        template<typename TIME>
        class dynamic_coupled : model {

            models_map _models;
            ports_vector _input_ports;
            ports_vector _output_ports;
            EC_vector _eic;
            EC_vector _eoc;
            IC_vector _ic;

        public:
            dynamic_coupled(
                    models_map models,
                    ports_vector input_ports,
                    ports_vector output_ports,
                    EC_vector eic,
                    EC_vector eoc,
                    IC_vector ic
            ) {
                assert(valid_ic_links(models, ic));
                assert(valid_eic_links(models, input_ports, eic));
                assert(valid_eoc_links(models, output_ports, eoc));

                _models = models;
                _input_ports = input_ports;
                _output_ports = output_ports;
                _eic = eic;
                _eoc = eoc;
                _ic = ic;
            }

            dynamic_coupled(
                    initializer_list_models_map models,
                    initilizer_list_ports_vector input_ports,
                    initilizer_list_ports_vector output_ports,
                    initializer_list_EC_vector eic,
                    initializer_list_EC_vector eoc,
                    initializer_list_IC_vector ic
            ) {
                _models = models;
                _input_ports = input_ports;
                _output_ports = output_ports;
                _eic = eic;
                _eoc = eoc;
                _ic = ic;

                assert(valid_ic_links(_models, _ic));
                assert(valid_eic_links(_models, _input_ports, _eic));
                assert(valid_eoc_links(_models, _output_ports, _eoc));
            }
        };

    }
}

#endif //CADMIUM_DYNAMIC_COUPLED_HPP
