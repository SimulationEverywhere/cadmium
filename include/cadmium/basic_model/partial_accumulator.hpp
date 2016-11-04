/**
 * Copyright (c) 2013-2016, 
 * Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis
 * Cristina Ruiz Martin
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
     * - S = {Numeric:total, bool:reseted, bool:partial}
     * - internal({total, ?, ?}) = {0, false, false}
     * - external({total, b, i}, t, x.add) = {total+x, b, i}
     *   external({total, b, i}, t, x.reset) = {total, true, i}
     *   external({total, b, i}, t, x.partial) = {total, b, true}
     *   external({total, b, i}, t, x.add & x.reset) = {total+x, true, i}
     *   external({total, b, i}, t, x.add & x.partial) = {total+x, b, true}
     *   external({total, b, i}, t, x.add & x.reset & x.partial) = {total+x, true, true}
     *   external({total, b, i}, t, x.reset & x.partial) = {total, true, true}
     * - confluence({total, b, i}, 0, x) = external(internal({total, b, i}), 0, x)
     * - output ({total, b, true}) = outport:{total}
     * - time_advance({total, ?, true}) = 0
     *   time_advance({total, true, ?}) = 0
     *   time_advance ({total, false, false}) = infinite
    */

    template<typename VALUE, typename SET> //VALUE is the type of accumulated values, SET is any type 
      struct partial_accumulator_defs {
        struct sum : public out_port<VALUE> {
        };
        struct add : public in_port<VALUE> {
        };
        struct reset : public in_port<SET> {
        };
        struct partial : public in_port<SET>{
        };
    };   

    template<typename VALUE, typename SET, typename TIME> //VALUE is the type of accumulated values, SET is any type 
      struct partial_accumulator {

        struct state_type{ 
          state_type(){
            on_reset = false;
            on_partial = false;
            accumulated = VALUE{};
          }
          bool on_reset;
          bool on_partial;
          VALUE accumulated;

          void set_state(VALUE accum, bool reset, bool partial){
            accumulated = accum;
            on_reset = reset;
            on_partial = partial;            
          }
        };

        using defs = partial_accumulator_defs<VALUE, SET>;

        state_type state = state_type();

        //default constructor
        constexpr partial_accumulator() noexcept {}

        //ports_definition
        using input_ports=std::tuple<typename defs::add, typename defs::reset, typename defs::partial>;
        using output_ports=std::tuple<typename defs::sum>;

        // PDEVS functions
        void internal_transition() {
          if(!state.on_reset && ! state.on_partial) {
            throw std::logic_error("Internal transition called while not on reset or not on partial state");
          }
          if(state.on_reset) {
            state.accumulated = VALUE{0};
            state.on_reset = false;
          }
          if(state.on_partial) {
            state.on_partial = false;
          }                
        }

        void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
          if(state.on_reset || state.on_partial) {
            throw std::logic_error("External transition called while on reset or on partial state");
          }
          for (const auto &x : get_messages<typename defs::add>(mbs)) {
            state.accumulated += x;
          }
          if (!get_messages<typename defs::partial>(mbs).empty()) state.on_partial = true; //multiple call equal one call
          if (!get_messages<typename defs::reset>(mbs).empty()) state.on_reset = true; //multiple call equal one call         
        }

        void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
          //process internal transition first
          internal_transition();
          //then external transition
          external_transition(0, std::move(mbs));
        }

        typename make_message_bags<output_ports>::type output() {       
          typename make_message_bags<output_ports>::type outmb;
          if(state.on_partial) {
            get_messages<typename defs::sum>(outmb).emplace_back(state.accumulated);
          }
          return outmb;
        }

        TIME time_advance() {
          //we assume default constructor of time is 0 and infinity is defined in numeric_limits
          TIME advace_time;
          if(state.on_reset || state.on_partial){
            advace_time = TIME{0};
          }else{
            advace_time = std::numeric_limits<TIME>::infinity();
          }
          return (advace_time);
        }
      };
    }
}

#endif // CADMIUM_PARTIAL_ACCUMULATOR_HPP