/**
 * Copyright (c) 2019, Damian Vicino
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


#ifndef CADMIUM_PDEVS_FILTER_FIRST_OUTPUT_HPP
#define CADMIUM_PDEVS_FILTER_FIRST_OUTPUT_HPP

#include<cadmium/modeling/ports.hpp>
#include<cadmium/modeling/message_bag.hpp>
#include<limits>
#include<cassert>


namespace cadmium::basic_models::pdevs {

    /**
     * @brief filter_first_output PDEVS Model used for testing a bug reported in 2017
     *
     * The model starts in passive state
     * When first exogenous message is received a 1 is output immediately
     * All following messages are discarded
     * For the purpose of test all messages are integers
     */

    struct filter_first_output_defs {
        //ports
        struct in : public in_port<int> {
        };
        struct out : public out_port<int> {
        };
    };


    template<typename TIME>
    class filter_first_output {
        using defs=filter_first_output_defs;// putting definitions in context
    public:
        //state
        using state_type=int;
        state_type state = 0;

        //default constructor
        constexpr filter_first_output() noexcept {}

        //ports_definition
        using input_ports=std::tuple<typename defs::in>;
        using output_ports=std::tuple<typename defs::out>;

        // PDEVS functions
        void internal_transition() {
            state++;
        }

        void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
            state++;
        }

        void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
            assert(false); //test should not call confluence
        }

        typename make_message_bags<output_ports>::type output() const {
            typename make_message_bags<output_ports>::type outmb;
            get_messages<defs::out>(outmb).emplace_back(1);
            return outmb;
        }

        TIME time_advance() const {
            return (state == 1 ? TIME{} : std::numeric_limits<TIME>::infinity());
        }
    };
}


#endif // CADMIUM_PDEVS_FILTER_FIRST_OUTPUT_HPP
