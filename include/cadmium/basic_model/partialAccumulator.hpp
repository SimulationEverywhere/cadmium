/**
 * Copyright (c) 2013-2016, Damian Vicino
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


#ifndef CADMIUM_PARTIAL_ACCUMULATOR_HPP
#define CADMIUM_PARTIAL_ACCUMULATOR_HPP
#include<cadmium/modeling/ports.hpp>
#include<cadmium/modeling/message_bag.hpp>
#include<limits>


namespace cadmium {
    namespace basic_models {

/**
 * @brief PartialAccumulator PDEVS Model.
 *
 * PartialAccumulator PDEVS Model:
 * - X = Numeric
 * - Y = Numeric
 * - In_Ports: add<Numeric>, reset, partial
 * - Out_Ports: outport
 * - S = {Numeric:total, bool:reseted, int:partial}
 * - internal({total, true, i}) = {0, false, 0}
 *   internal({total, false, 1}) = {total, false, 0}
 * - external({total, b, i}, t, x.add) = {total+x, b, i}
 *   external({total, b, i}, t, x.reset) = {total, true, i}
 *   external({total, b, i}, t, x.partial) = {total, b, 1}
 * - confluence({total, b, i}, 0, x) = external(internal({total, b, i}), 0, x)
 * - output ({total, b, 1}) = outport:{total}
 * - time_advance({total, b, 1}) = 0
 *   time_advance({total, true, i}) = 0
 *   time_advance ({total, false, 0}) = infinite
*/

        struct reset_tick {
        };

        struct set_partial{
        };

        template<typename VALUE, typename TIME> //value is the type of accumulated values
        struct partialAccumulator {
            //local definitions
            using value_type=VALUE;
            using on_reset=bool;
            using on_partial=int;

            //ports
            struct sum : public out_port<VALUE> {
            };
            struct add : public in_port<VALUE> {
            };
            struct reset : public in_port<reset_tick> {
            };
            struct partial : public in_port<set_partial>{
            };

            //required definitions start here
            //state
            using state_type=std::tuple<VALUE, on_reset, on_partial>;
            state_type state = std::make_tuple(VALUE{}, false, 0);

            //default constructor
            constexpr partialAccumulator() noexcept {}

            //ports_definition
            using input_ports=std::tuple<add, reset, partial>;
            using output_ports=std::tuple<sum>;

            // PDEVS functions
            void internal_transition() {
                if(!std::get<on_reset>(state) && std::get<on_partial>(state)!=1) {
                    throw std::logic_error("Internal transition called while not on reset or not on partial state");
                }

                if(std::get<on_reset>(state)) {
                    std::get<VALUE>(state) = VALUE{0};
                    std::get<on_reset>(state) = false;
                }

                if(std::get<on_partial>(state)==1) {
                    std::get<on_partial>(state) =0;
                }
                
            }

            void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
                if(std::get<on_reset>(state) || std::get<on_partial>(state)==1) {
                    throw std::logic_error("External transition called while on reset or on partial state");
                }

                //one message bag parameter for each port is received
                for (const auto &x : get_messages<add>(mbs)) {
                    std::get<VALUE>(state) += x;
                }
                if (!get_messages<reset>(mbs).empty())
                    std::get<on_reset>(state) = true; //multiple call equal one call

                if (!get_messages<partial>(mbs).empty())
                    std::get<on_partial>(state) = 1; //multiple call equal one call

            }

            void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
                //process internal transition first
                internal_transition();
                //then external transition
                external_transition(0, std::move(mbs));
            }

            typename make_message_bags<output_ports>::type output() {
                

                typename make_message_bags<output_ports>::type outmb;

                if(std::get<on_partial>(state)==1) {
                    get_messages<sum>(outmb).emplace_back(std::get<VALUE>(state));
                } else{ //we can reach the external because we are reseting
                    get_messages<sum>(outmb).clear();
                }
                return outmb;
            }

            TIME time_advance() {
                //we assume default constructor of time is 0 and infinity is defined in numeric_limits
                TIME advace_time;
                if(std::get<on_reset>(state) || std::get<on_partial>(state)==1){
                    advace_time = 0;
                }else{
                    advace_time = std::numeric_limits<TIME>::infinity();
                }

                return (advace_time);
            }
        };

    }
}


#endif // CADMIUM_PARTIAL_ACCUMULATOR_HPP