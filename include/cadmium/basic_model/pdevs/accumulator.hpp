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


#ifndef CADMIUM_PDEVS_ACCUMULATOR_HPP
#define CADMIUM_PDEVS_ACCUMULATOR_HPP

#include<limits>

#include<cadmium/modeling/ports.hpp>
#include<cadmium/modeling/message_bag.hpp>
#include<cadmium/logger/tuple_to_ostream.hpp> // included to allow the accumulator state to use the << operator


namespace cadmium::basic_models::pdevs {

/**
 * @brief Accumulator PDEVS Model.
 *
 * Accumulator PDEVS Model:
 * - In_Ports: add<Numeric>, reset
 * - Out_Ports: outport
 * - S = {Numeric:total, bool:reseted}
 * - internal({total, true}, 0) = {0, false}
 * - external({total, b}, t, x) = {total+x, b}
 * - confluence({total, b}, 0, x) = external(internal({total, true}, 0), 0, x)
 * - output ({total, b}) = outport:{total}
 * - time_advance({total, true}) = 0
 *   time_advance({total, false}) = infinite
*/

    //definitions used for defining the accumulator that need to be accessed by externals resources before instantiate the models
    template<typename VALUE>
    struct accumulator_defs {
        //custom messages
        struct reset_tick {
        };
        //custom ports
        struct sum : public out_port<VALUE> {
        };
        struct add : public in_port<VALUE> {
        };
        struct reset : public in_port<reset_tick> {
        };
    };

    template<typename VALUE, typename TIME> //value is the type of accumulated values
    class accumulator {
        using defs=accumulator_defs<VALUE>;// putting definitions in context
    public:
        //state
        using on_reset=bool;
        using state_type=std::tuple<VALUE, on_reset>;
        state_type state = std::make_tuple(VALUE{}, false);

        //default constructor
        constexpr accumulator() noexcept {}

        //ports_definition
        using input_ports=std::tuple<typename defs::add, typename defs::reset>;
        using output_ports=std::tuple<typename defs::sum>;

        // PDEVS functions
        void internal_transition() {
            if (!std::get<on_reset>(state)) {
                throw std::logic_error("Internal transition called while not on reset state");
            }
            std::get<VALUE>(state) = VALUE{0};
            std::get<on_reset>(state) = false;
        }

        void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
            if (std::get<on_reset>(state)) {
                throw std::logic_error("External transition called while on reset state");
            }

            //one message bag parameter for each port is received
            for (const auto &x : get_messages<typename defs::add>(mbs)) {
                std::get<VALUE>(state) += x;
            }
            if (!get_messages<typename defs::reset>(mbs).empty())
                std::get<on_reset>(state) = true; //multiple call equal one call
        }

        void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
            //process internal transition first
            internal_transition();
            //then external transition
            //we assume the default constructor of TIME produces a zero
            external_transition(TIME{}, std::move(mbs));
        }

        typename make_message_bags<output_ports>::type output() const {
            if (!std::get<on_reset>(state)) {
                throw std::logic_error("Output function called while not on reset state");
            }

            typename make_message_bags<output_ports>::type outmb;
            get_messages<typename defs::sum>(outmb).emplace_back(std::get<VALUE>(state));
            return outmb;
        }

        TIME time_advance() const {
            //we assume default constructor of time is 0 and infinity is defined in numeric_limits
            return (std::get<on_reset>(state) ? TIME{} : std::numeric_limits<TIME>::infinity());
        }
    };
}

#endif //CADMIUM_PDEVS_ACCUMULATOR_HPP
