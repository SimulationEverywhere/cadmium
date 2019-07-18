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

#ifndef CADMIUM_COUPLED_MODEL_ASSERT_HPP
#define CADMIUM_COUPLED_MODEL_ASSERT_HPP

#include<cadmium/concept/concept_helpers.hpp>
#include<cadmium/modeling/message_bag.hpp>
#include<cadmium/concept/atomic_model_assert.hpp>
#include<cadmium/modeling/ports.hpp>
#include<type_traits>

namespace cadmium::concept {
    //static assert over the EIC descriptions
    template<typename IN, typename EICs, int S>
    struct assert_each_eic {
        static constexpr bool value() {
            using EP=typename std::tuple_element<S - 1, EICs>::type::external_input_port;
            using SUBMODEL=typename std::tuple_element<S -
                                                       1, EICs>::type::template submodel<float>; //using float time, ports should not depend on time
            using IP=typename std::tuple_element<S - 1, EICs>::type::submodel_input_port;
            //ports are input ports
            static_assert(EP::kind == port_kind::in, "The external port in a EIC is not an input port");
            static_assert(IP::kind == port_kind::in, "The internal port in a EIC is not an input port");
            //check internal port is defined in submodel
            static_assert(has_port_in_tuple<IP, typename SUBMODEL::input_ports>::value(),
                          "External port in EIC is not defined as external port in the model");
            //check external port is defined in the model
            static_assert(has_port_in_tuple<EP, IN>::value(),
                          "External port in EIC is not defined as external port in the model");
            //check the internal port matches message type with the external port
            static_assert(std::is_same<typename EP::message_type, typename IP::message_type>(),
                          "The message type does not match in EIC description");

            //recursive step
            return assert_each_eic<IN, EICs, S - 1>::value();
        }
    };

    template<typename IN, typename EICs>
    struct assert_each_eic<IN, EICs, 0> {
        static constexpr bool value() {
            return true;
        }
    };


    template<typename IN, typename EICs>
    constexpr void assert_eic(IN p_in, EICs eics) {
        assert_each_eic<IN, EICs, std::tuple_size<EICs>::value>::value();//check couple individually
        static_assert(is_tuple<EICs>(), "EIC is not a tuple");
    }

    //static assert over the EOC descriptions
    template<typename OUT, typename EOCs, int S>
    struct assert_each_eoc {
        static constexpr bool value() {
            using EP=typename std::tuple_element<S - 1, EOCs>::type::external_output_port;
            using SUBMODEL=typename std::tuple_element<S -
                                                       1, EOCs>::type::template submodel<float>; //using float time, ports should not depend on time
            using IP=typename std::tuple_element<S - 1, EOCs>::type::submodel_output_port;
            //ports are output ports
            static_assert(EP::kind == port_kind::out, "The external port in a EOC is not an output port");
            static_assert(IP::kind == port_kind::out, "The internal port in a EOC is not an output port");
            //check internal port is defined in submodel
            static_assert(has_port_in_tuple<IP, typename SUBMODEL::output_ports>::value(),
                          "Internal port in EOC is not defined as external port in the submodel");
            //check external port is defined in the model
            static_assert(has_port_in_tuple<EP, OUT>::value(),
                          "External port in EIC is not defined as external port in the model");
            //check the internal port matches message type with the external port
            static_assert(std::is_same<typename EP::message_type, typename IP::message_type>(),
                          "The message type does not match in EOC description");

            //recursive step
            return assert_each_eoc<OUT, EOCs, S - 1>::value();
        }
    };

    template<typename OUT, typename EOCs>
    struct assert_each_eoc<OUT, EOCs, 0> {
        static constexpr bool value() {
            return true;
        }
    };


    template<typename OUT, typename EOCs>
    constexpr void assert_eoc(OUT p_out, EOCs eocs) {
        assert_each_eoc<OUT, EOCs, std::tuple_size<EOCs>::value>::value();//check couple individually
        static_assert(is_tuple<EOCs>(), "EOC is not a tuple");
    }

    //static assert over the IC descriptions
    template<typename ICs, int S>
    struct assert_each_ic {
        static constexpr bool value() {
            using FROM_MODEL=typename std::tuple_element<S -
                                                         1, ICs>::type::template from_model<float>;//using float time, ports should not depend on time
            using FROM_PORT=typename std::tuple_element<S - 1, ICs>::type::from_model_output_port;
            using TO_MODEL=typename std::tuple_element<
                    S - 1, ICs>::type::template to_model<float>; //using float time, ports should not depend on time
            using TO_PORT=typename std::tuple_element<S - 1, ICs>::type::to_model_input_port;

            //ports are proper kind
            static_assert(FROM_PORT::kind == port_kind::out,
                          "The port in from_model in a IC is not an output port");
            static_assert(TO_PORT::kind == port_kind::in, "The port in a to_model in a IC is not an input port");

            //check ports are defined in submodels
            static_assert(has_port_in_tuple<FROM_PORT, typename FROM_MODEL::output_ports>::value(),
                          "Output port used in IC is not defined in the submodel");
            static_assert(has_port_in_tuple<TO_PORT, typename TO_MODEL::input_ports>::value(),
                          "Input port in IC is not defined in the submodel");
            //check the internal port matches message type with the external port
            static_assert(std::is_same<typename TO_PORT::message_type, typename FROM_PORT::message_type>(),
                          "The message type does not match in IC description");

            //not loop in IC
            static_assert(!std::is_same<FROM_MODEL, TO_MODEL>(), "The IC detected a coupling-to-self loop");

            //recursive step
            return assert_each_ic<ICs, S - 1>::value();
        }
    };

    template<typename ICs>
    struct assert_each_ic<ICs, 0> {
        static constexpr bool value() {
            return true;
        }
    };


    template<typename ICs>
    constexpr void assert_ic(ICs ics) {
        assert_each_ic<ICs, std::tuple_size<ICs>::value>::value();//check couple individually
        static_assert(is_tuple<ICs>(), "ICs is not a tuple");
    }

    namespace pdevs {
        //forward declaration
        template<typename MODEL>
        constexpr void coupled_model_float_time_assert();

        //asserting the submodels are proper coupled or atomic modes.
        template<typename MODELs, int S>
        struct assert_each_submodel {
            using MODEL=typename std::tuple_element<S - 1, MODELs>::type;
            //testing if model has to be checked as atomic or coupled
            using atomic_detected=char;
            using coupled_detected=long;

            template<typename M>
            static atomic_detected test(decltype(&M::time_advance));

            template<typename M>
            static coupled_detected test(...);

            static constexpr void check() {
                if constexpr (sizeof(test<MODEL>(0)) == sizeof(atomic_detected)) {
                    cadmium::concept::pdevs::atomic_model_float_time_assert<MODEL>();
                } else {
                    coupled_model_float_time_assert<MODEL>();
                }
                assert_each_submodel<MODELs, S - 1>();
            }
        };

        template<typename MODELs>
        struct assert_each_submodel<MODELs, 0> {
            static constexpr void check() {}
        };

        template<typename MODELs>
        constexpr void assert_submodels() {
            static_assert(is_tuple<MODELs>(), "Submodels is not a tuple");
            assert_each_submodel<MODELs, std::tuple_size<MODELs>::value>::check();//check submodels models individually
        }

        //coupled model full assert check
        template<typename FLOATING_MODEL>
        constexpr void coupled_model_float_time_assert() {
            using IP=typename FLOATING_MODEL::input_ports;
            using OP=typename FLOATING_MODEL::output_ports;
            using EICs=typename FLOATING_MODEL::external_input_couplings;
            using EOCs=typename FLOATING_MODEL::external_output_couplings;
            using ICs=typename FLOATING_MODEL::internal_couplings;
            //check EICs are EICs connecting ports of same message type
            assert_eic(IP{}, EICs{});
            //check EOCs are EOCs connecting ports of same message type.
            assert_eoc(OP{}, EOCs{});
            //check ICs are ICs connecting ports of same message type.
            assert_ic(ICs{});
            //check port types are unique for in the portset tuples
            static_assert(check_unique_elem_types<IP>::value(), "ambiguous port name in input ports");
            static_assert(check_unique_elem_types<OP>::value(), "ambiguous port name in output ports");
            //check submodels when using float as TIME
            using floating_submodels=typename FLOATING_MODEL::template models<float>;
            static_assert(std::is_constructible<floating_submodels>::value, "coupled model cannot be constructed");
            assert_submodels<floating_submodels>();
        }

        template<template<typename TIME> class MODEL>
        //check a template argument is required (for time)
        constexpr void coupled_model_assert() {
            using floating_model=MODEL<float>;
            coupled_model_float_time_assert<floating_model>();
        }

    }
}
#endif // CADMIUM_COUPLED_MODEL_ASSERT_HPP
