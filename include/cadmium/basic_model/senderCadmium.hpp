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
* SENDERCadmium:
* Cadmium implementation of CD++ SENDER atomic model
*/

#ifndef CADMIUM_SENDER_HPP
#define CADMIUM_SENDER_HPP
#include<cadmium/modeling/ports.hpp>
#include<cadmium/modeling/message_bag.hpp>
#include<limits>

using namespace std;

namespace cadmium {
  namespace basic_models {

  /**
   * @brief SENDER PDEVS Model.
   *
  */

    template<typename VALUE, typename TIME> //value is the type of accumulated values
      struct sender {
            
      struct sender_state{

        sender_state(bool ac, int p, int tp, int ab, bool s, bool a, TIME n){
          ack = ac;
          packetNum = p;
          totalPacketNum = tp;
          alt_bit = ab;
          sending = s;
          active = a;
          next_internal = n;

        }
        bool ack;
        int packetNum;
        int totalPacketNum;
        int alt_bit;
        bool sending;
        bool active;
        TIME next_internal;
      };
  
      //ports
      struct dataOut : public out_port<VALUE> {
      };
      struct ackReceived : public out_port<VALUE> {
      };
      struct packetSent : public out_port<VALUE> {
      };
      struct controlIn : public in_port<VALUE> {
      };
      struct ackIn : public in_port<VALUE> {
      };
  
      //required definitions start here
      //state
      using state_type=sender_state;
      state_type state = state_type(false, 0, 0, 0, false, false, std::numeric_limits<TIME>::infinity());
  

  
      //PARAMETRES
      string id = string("Sender");  
      TIME  preparationTime = 10;
      TIME timeout = 20;
  
      //default constructor
      constexpr sender() noexcept {}

      //ports_definition
      using input_ports=std::tuple<controlIn, ackIn>;
      using output_ports=std::tuple<dataOut, ackReceived, packetSent>;
  
      // PDEVS functions
      void internal_transition() {
        if (state.ack){
          if (state.packetNum < state.totalPacketNum){
            state.packetNum ++;
            state.ack = false;
            state.alt_bit = (state.alt_bit + 1) % 2;
            state.sending = true;
            state.active = true; 
            state.next_internal = preparationTime;   
          } else {
            state.active = false;
            state.next_internal = std::numeric_limits<TIME>::infinity();
          }
        } else{
          if (state.sending){
            state.sending = false;
            state.active = true;
            state.next_internal = timeout;
          } else {
            state.sending = true;
            state.active = true;
            state.next_internal = preparationTime;    
          } 
        }                  
      }
  
      void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
         
        assert(get_messages<controlIn>(mbs).size()==1 && "Only one message at a time");
        assert(get_messages<ackIn>(mbs).size()==1 && "Only one message at a time");
        
        for (const auto &x : get_messages<controlIn>(mbs)) {
          if(state.active == false){
            state.totalPacketNum = static_cast < int > (x);
            if (state.totalPacketNum > 0){
              state.packetNum = 1;
              state.ack = false;
              state.sending = true;
              state.alt_bit = state.packetNum % 2;  //set initial alt_bit
              state.active = true;
              state.next_internal = preparationTime;
            }else{
              if(state.next_internal != std::numeric_limits<TIME>::infinity()){
                state.next_internal = state.next_internal - e;
              }
            }
          }
        }

        for (const auto &x : get_messages<ackIn>(mbs)) {
          if(state.active == true){
            if (state.alt_bit == static_cast < int > (x)){
              state.ack = true;
              state.sending = false;
              state.next_internal = 0;
            }else{
              if(state.next_internal != std::numeric_limits<TIME>::infinity()){
                state.next_internal = state.next_internal - e;
              }
            }
          }
        }
      }   
  
      void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        external_transition(e, std::move(mbs));
      }
  
      typename make_message_bags<output_ports>::type output() {             
  
        typename make_message_bags<output_ports>::type outmb;

        if (state.sending){
          get_messages<dataOut>(outmb).emplace_back(state.packetNum * 10 + state.alt_bit);
          get_messages<packetSent>(outmb).emplace_back(state.packetNum);
        }else{
          if (state.ack) get_messages<ackReceived>(outmb).emplace_back(state.alt_bit);
        }   
        
        return outmb;
      }
  
      TIME time_advance() {
        return state.next_internal;
      }

    };

  }
}

#endif // CADMIUM_SENDER_HPP