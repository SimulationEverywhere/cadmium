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

#ifndef CADMIUM_COUPLING_HPP
#define CADMIUM_COUPLING_HPP

#include<type_traits>
#include<tuple>

namespace cadmium::modeling {

    /**
     * model_tuple is a struct used to keep the TIME as template parameter for the models being coupled
     */
    template<template<typename TIME> class... Ms>
    struct models_tuple {
        template<typename T>
        using type=std::tuple<Ms<T>...>;
    };

    /**
     * EIC is a coupling from an input port in the coupled model to an input port of a sub model
     */
    template<typename EXTERNAL_PORT, template<typename TIME> class SUBMODEL, typename SUBMODEL_PORT>
    struct EIC {
        using external_input_port=EXTERNAL_PORT;
        template<typename TIME>
        using submodel=SUBMODEL<TIME>;
        using submodel_input_port=SUBMODEL_PORT;
    };

    /**
     * EOC is a coupling from a submodel output port to an outpit port in the coupled model
     */
    template<template<typename TIME> class SUBMODEL, typename SUBMODEL_PORT, typename EXTERNAL_PORT>
    struct EOC {
        template<typename TIME>
        using submodel=SUBMODEL<TIME>;
        using submodel_output_port=SUBMODEL_PORT;
        using external_output_port=EXTERNAL_PORT;
    };

    /**
     * IC is a coupling from a submodel output port to an output port in the coupled model
     */
    template<template<typename TIME> class SUBMODEL_FROM, typename PORT_FROM,
            template<typename TIME> class SUBMODEL_TO, typename PORT_TO>
    struct IC {
        template<typename TIME>
        using from_model=SUBMODEL_FROM<TIME>;
        using from_model_output_port=PORT_FROM;
        template<typename TIME>
        using to_model=SUBMODEL_TO<TIME>;
        using to_model_input_port=PORT_TO;
    };


    namespace pdevs {
        /**
         * A couple model is a list of submodels, input ports, output ports and couplings EICs, ICs, EOCs.
         * Template parameter TIME is not required for coupled models, because they do not produce behavior
         * since coupled model are structural definitions only.
         *
         * IP are a tuple of input_port
         * OP are a tuple of output_port
         * MODELS a models_tuple (the struct is a helper for simplifying the definition of tuple of templates)
         * EICs is a tuple of EIC coupling input ports of the coupled model to input ports in submodels
         * EOCs is a tuple of EOC coupling output ports in submodels to output ports of the coupled model
         * ICs is a tuple IC coupling output ports in submodels to input ports in submodels
         */
        template<typename TIME, typename IP, typename OP, class Ms, typename EICs, typename EOCs, typename ICs>
        struct coupled_model {
            using input_ports=IP;
            using output_ports=OP;
            template<typename T>
            using models=typename Ms::template type<T>;
            using external_input_couplings=EICs;
            using external_output_couplings=EOCs;
            using internal_couplings=ICs;
        };
    }

    namespace devs {
        /**
         * A couple model is a list of submodels, input ports, output ports and couplings EICs, ICs, EOCs.
         * Template parameter TIME is not required for coupled models, because they do not produce behavior
         * since coupled model are structural definitions only.
         *
         * IP are a tuple of input_port
         * OP are a tuple of output_port
         * MODELS a models_tuple (the struct is a helper for simplifying the definition of tuple of templates)
         * EICs is a tuple of EIC coupling input ports of the coupled model to input ports in submodels
         * EOCs is a tuple of EOC coupling output ports in submodels to output ports of the coupled model
         * ICs is a tuple IC coupling output ports in submodels to input ports in submodels
         * SELECT is a template class with an imminent function that, given a list of imminent candidate
         * models returns the imminent.
         */
        template<typename TIME,
                typename IP,
                typename OP,
                class Ms,
                typename EICs,
                typename EOCs,
                typename ICs,
                typename SELECT>
        struct coupling {
            using input_ports=IP;
            using output_ports=OP;
            template<typename T>
            using models=typename Ms::template type<T>;
            using external_input_couplings=EICs;
            using external_output_couplings=EOCs;
            using internal_couplings=ICs;
            using select=SELECT;
        };

    }

}


#endif //CADMIUM_COUPLING_HPP

