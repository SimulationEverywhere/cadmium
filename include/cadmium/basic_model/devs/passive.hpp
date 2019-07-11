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

#ifndef CADMIUM_DEVS_PASSIVE_HPP
#define CADMIUM_DEVS_PASSIVE_HPP

#include<cadmium/modeling/ports.hpp>
#include<cadmium/modeling/message_box.hpp>
#include<limits>

namespace cadmium::basic_models::devs {


/**
 * @brief Passive PDEVS Model receives messages of a particular type and never outputs.
 *
 * Passive PDEVS Model:
 * - X = any (of particular type in implementation)
 * - Y = void
 * - In_Ports: in
 * - Out_Ports: out
 * - S = {}
 * - internal( ., .) = ERROR
 * - external( ., ., .) = {}
 * - confluence( ., ., .) = ERROR
 * - output (.) = ERROR
 * - time_advance(.) = infinite
*/

    //definitions used for defining the accumulator that need to be accessed by externals resources before instantiate the models
    //This includes Ports referenced by couplings, and
    template<typename VALUE>
    struct passive_defs {
        //custom ports
        struct in : public in_port<VALUE> {
        };
    };


    template<typename VALUE, typename TIME> //VALUE is the type of X
    class passive {
        using defs=passive_defs<VALUE>;// putting definitions in context
    public:
        // required definitions start here
        // default constructor
        constexpr passive() noexcept {}

        // state definition
        using state_type=int; //A type has to be declared and void is not allowed for variables
        state_type state = 0;

        // ports definition
        using input_ports=std::tuple<typename defs::in>;
        using output_ports=std::tuple<>; //no output ports;

        // internal transition should never be run
        void internal_transition() {
            throw std::logic_error("Internal transition called in a passive model");
        }

        // external transition should do nothing
        void external_transition(TIME e, typename make_message_box<input_ports>::type mb) {
        }

        // output function should never be run
        typename make_message_box<output_ports>::type output() const {
            throw std::logic_error("Output function called in a passive model");
        }

        // time_advance should always return infinity
        TIME time_advance() const {
            //we assume default constructor of TIME is 0 and infinity is defined in numeric_limits
            return (std::numeric_limits<TIME>::infinity());
        }
    };
}

#endif // CADMIUM_DEVS_PASSIVE_HPP
