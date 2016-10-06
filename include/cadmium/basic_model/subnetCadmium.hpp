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

/*
* SUBNETCadmium:
* Cadmium implementation of CD++ SUBNET atomic model
*/

#ifndef CADMIUM_SUBNET_HPP
#define CADMIUM_SUBNET_HPP
#include<cadmium/modeling/ports.hpp>
#include<cadmium/modeling/message_bag.hpp>
#include<limits>

using namespace std;

namespace cadmium {
  namespace basic_models {

  /**
   * @brief SUBNET PDEVS Model.
   *
  */

    template<typename VALUE, typename TIME> //value is the type of accumulated values
      struct subnet {
            
      struct subnet_state{
        subnet_state(bool a, int p, int i){
          active = a;
          packet = p;
          index = i;
        }
        bool active;
        int packet;
        int index;
      };
  
      //ports
      struct out : public out_port<VALUE> {
      };
      struct in : public in_port<VALUE> {
      };
  
      //required definitions start here
      //state
      using state_type=subnet_state;
      state_type state = state_type(false, 0, 0);
  
      //PARAMETRES
      string id = string("Subnet");  
  
      //default constructor
      constexpr subnet() noexcept {}

      //ports_definition
      using input_ports=std::tuple<in>;
      using output_ports=std::tuple<out>;
  
      // PDEVS functions
      void internal_transition() {
        state.active = false;               
      }
  
      void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
         
        assert(get_messages<in>(mbs).size()==1 && "Only one message at a time");
        state.index ++;
        
        for (const auto &x : get_messages<in>(mbs)) {
          state.packet = static_cast < int > (x);
          state.active = true;
        }
      }   
  
      void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        external_transition(e, std::move(mbs));
      }
  
      typename make_message_bags<output_ports>::type output() {             
  
        typename make_message_bags<output_ports>::type outmb;

        if ((double)rand() / (double) RAND_MAX  < 0.95){
          get_messages<out>(outmb).emplace_back(state.packet);
        }   
        
        return outmb;
      }
  
      TIME time_advance() {
        std::default_random_engine generator;
        std::normal_distribution<double> distribution(3.0, 1.0); 
        TIME next_internal;
        if (state) {
          next_internal = TIME(static_cast < int > (round(distribution(generator))));
        }else {
          next_internal = std::numeric_limits<TIME>::infinity();
        }    
        return next_internal;
      }

    };

  }
}

#endif // CADMIUM_SUBNET_HPP