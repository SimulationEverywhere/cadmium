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


#ifndef CADMIUM_PDEVS_GENERATOR_ONE_HPP
#define CADMIUM_PDEVS_GENERATOR_ONE_HPP

#include<cadmium/modeling/ports.hpp>
#include<cadmium/modeling/message_bag.hpp>
#include<limits>

namespace cadmium::basic_models::pdevs {
    /**
     * @brief Generator PDEVS Model
     *
     * Generator of integers every 1 sec PDEVS Model:
     * - X = {}
     * - Y = {1}
     * - S = {passive, active} x Multiples(1)
     * - internal(phase, t) = ("active", 1)
     * - external = {}
     * - out ("active", t) = outvalue
     * - advance(phase, t) = 1 - t
     */

    //definitions used for defining the accumulator that need to be accessed by externals resources before instantiate the models
    //This includes Ports referenced by couplings, and

    struct int_generator_one_sec_defs {
        //custom ports
        struct out : public out_port<int> {
        };
    };


    template<typename TIME> //VALUE is the type of Y
    class int_generator_one_sec {
        using defs=int_generator_one_sec_defs;// putting definitions in context
    public:
        //these functions need to be overriden to define the generator behavior
        TIME period() const { // time between consecutive messages
            return 1.0;
        }

        int output_message() const {// message to be output
            return 1;
        }

        // required definitions start here
        // default constructor
        constexpr int_generator_one_sec() noexcept {}

        // state definition
        using state_type=int;
        state_type state = 0;

        // ports definition
        using input_ports=std::tuple<>;
        using output_ports=std::tuple<typename defs::out>;

        // internal transition
        void internal_transition() {}

        // external transition
        void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
            throw std::logic_error("External transition called in a model with no input ports");
        }

        // confluence transition
        void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
            throw std::logic_error("Confluence transition called in a model with no input ports");
        }

        // output function
        typename make_message_bags<output_ports>::type output() const {
            typename make_message_bags<output_ports>::type bags;
            cadmium::get_messages<typename defs::out>(bags).push_back(output_message());
            return bags;
        }

        // time_advance function
        TIME time_advance() const {
            //we assume default constructor of TIME is 0 and infinity is defined in numeric_limits
            return period();
        }
    };
}

#endif //CADMIUM_PDEVS_GENERATOR_ONE_HPP

