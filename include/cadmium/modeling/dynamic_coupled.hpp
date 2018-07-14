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

#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_models_helpers.hpp>

namespace cadmium {
    namespace dynamic {
        namespace modeling {

            template<typename TIME>
            class coupled : public cadmium::dynamic::modeling::model {
            public:
                std::string _id;
                Models _models;
                Ports _input_ports;
                Ports _output_ports;
                EICs _eic;
                EOCs _eoc;
                ICs _ic;

                coupled() = delete;

                coupled(std::string id) : _id(id) {}

                coupled(
                        std::string id,
                        Models models,
                        Ports input_ports,
                        Ports output_ports,
                        EICs eic,
                        EOCs eoc,
                        ICs ic
                ) :
                        _id(id),
                        _models(models),
                        _input_ports(input_ports),
                        _output_ports(output_ports),
                        _eic(eic),
                        _eoc(eoc),
                        _ic(ic)
                {
                    if (!valid_ic_links(_models, _ic)) {
                        throw std::domain_error("Coupled model" + _id + " has invalid IC links");
                    }

                    if (!valid_eic_links(_models, _input_ports, _eic)) {
                        throw std::domain_error("Coupled model" + _id + " has invalid EIC links");
                    }

                    if(!valid_eoc_links(_models, _output_ports, _eoc)) {
                        throw std::domain_error("Coupled model" + _id + " has invalid EOC links");
                    }
                }

                coupled(
                        std::string id,
                        initializer_list_Models models,
                        initilizer_list_Ports input_ports,
                        initilizer_list_Ports output_ports,
                        initializer_list_EICs eic,
                        initializer_list_EOCs eoc,
                        initializer_list_ICs ic
                ) :
                        _id(id),
                        _models(models),
                        _input_ports(input_ports),
                        _output_ports(output_ports),
                        _eic(eic),
                        _eoc(eoc),
                        _ic(ic)
                {
                    if (!valid_ic_links(_models, _ic)) {
                        throw std::domain_error("Coupled model" + _id + " has invalid IC links");
                    }

                    if (!valid_eic_links(_models, _input_ports, _eic)) {
                        throw std::domain_error("Coupled model" + _id + " has invalid EIC links");
                    }

                    if(!valid_eoc_links(_models, _output_ports, _eoc)) {
                        throw std::domain_error("Coupled model" + _id + " has invalid EOC links");
                    }
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

            };
        }
    }
}

#endif //CADMIUM_DYNAMIC_COUPLED_HPP
