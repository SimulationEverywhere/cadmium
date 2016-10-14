/**
 * Copyright (c) 2013-2015, 
 * Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis
 * Cristina Ruiz Martín
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

#ifndef CADMIUM_PDEVS_ISTREAM_HPP
#define CADMIUM_PDEVS_ISTREAM_HPP

#include<cadmium/modeling/ports.hpp>
#include<cadmium/modeling/message_bag.hpp>
#include<limits>

namespace cadmium {
  namespace basic_models {

    /**
     * @brief input_stream PDEVS Model.
     *
     * istream PDEVS Model plays a history of events received by an input stream.
     * The list of events allows to be used as a connector to external tools.
     * The input format is "time output and a custom parser can be defined."
     *
    */
    
    template<typename TIME, typename MSG, typename T=int, typename M=int > //T and M are the type expected to be read from the ISTREAM
      struct input_stream {
        // port definition helper
        struct out : public out_port<MSG> {
        };
    
        struct state_variables{
    
          state_variables(){
          }
    
          std::shared_ptr<std::istream> ps; //the stream
          TIME last;
          TIME next;
          std::vector<MSG> output;
          TIME prefetched_time;
          MSG prefetched_message;
        };
          
        void (*_process)(const std::string&, TIME&, MSG&); //Parser process reads the string and sets the time,msg
    
    
        //helper function
        private:
        void fetchUntilTimeAdvances() {
            //making use of the prefetched values
            state.next = state.prefetched_time;
            state.output = {state.prefetched_message};
            //fetching next messages
            std::string line;
            do
                std::getline(*state.ps, line);
            while(!state.ps->eof() && line.empty());
            if (state.ps->eof() && line.empty()){
                //if there is no more messages, set infinity as next event time
                state.prefetched_time = std::numeric_limits<TIME>::infinity();
            } else { //else cache the las message fetched
                //intermediary vars for casting
                TIME t_next;
                MSG m_next;
                _process(line, t_next, m_next);
                //advance until different time is fetched
                while( state.next == t_next){
                    state.output.push_back(m_next);
                    line.clear();
                    std::getline(*state.ps, line);
                    if (state.ps->eof() && line.empty()){
                        state.prefetched_time = std::numeric_limits<TIME>::infinity();
                        return;
                    } else {
                        _process(line, t_next, m_next);
                    }
                }
                //cache the last message fetched
                if (t_next < state.next) throw std::exception();
                state.prefetched_time = t_next;
                state.prefetched_message = m_next;
            }
        }
    
        public:
    
        /**
        * @brief input_stream constructor sets the stream to be read and the initial time
        * @param pis is a pointer to the input stream to be read
        * @param init is the time the simulation of the model starts, the input MUST have absolute times greater than init time.
        */
    
        explicit input_stream(std::shared_ptr<std::istream> pis, TIME init) noexcept :
          input_stream(pis, init,
              [](const std::string& s, TIME& t_next, MSG& m_next){
                              T tmp_next;
                              M tmp_next_out;
                              std::stringstream ss;
                              ss.str(s);
                              ss >> tmp_next;
                              t_next = static_cast<TIME>(tmp_next);
                              ss >> tmp_next_out;
                              m_next = static_cast<MSG>(tmp_next_out);
                              std::string thrash;
                              ss >> thrash;
                              if ( 0 != thrash.size()) throw std::exception();
                          }
                      )
          {}
    
        /**
        * @brief input_stream constructor sets the stream to be read and the initial time and a custom parser
        * @param pis is a pointer to the input stream to be read
        * @param init is the time the simulation of the model starts, the input MUST have absolute times greater than init time.
        * @param process the process to parse each line of input and extract time and messages
        */
        explicit input_stream(std::shared_ptr<std::istream> pis, TIME init, decltype(_process) process)  noexcept {
            state.ps = pis;
            state.last = init;
            _process = process;
            std::string line;
            std::getline(*state.ps, line); //needs at least one call to detect eof
            if (state.ps->eof() && line.empty()){
                state.next = std::numeric_limits<TIME>::infinity();
            } else {
                //intermediary vars for casting
                TIME t_next;
                MSG m_next;
                process(line, t_next, m_next);
                //the first iteration needs this to run
                state.prefetched_time = t_next;
                state.prefetched_message = m_next;
                fetchUntilTimeAdvances();
            }
        }
    
        // state definition
        using state_type=state_variables; //A type has to be declared and void is not allowed for variables
        state_type state = state_type();
    
        // ports definition
        using input_ports=std::tuple<>;            
        using output_ports=std::tuple<out>;
    
        /**
        * @brief internal function reads the stream and prepares next event.
        */
        void internal_transition() {
            state.last = state.next;
            fetchUntilTimeAdvances();
        }
    
        /**
        * @brief invalid external function.
        */
        void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
          throw std::logic_error("Non external input is expected in this model");
        }
    
        /**
        * @brief invalid confluence function.
        */
        void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
            throw std::logic_error("Non external input is expected in this model");
        }
    
        /**
        * @brief out function.
        * @return the event defined in the input.
        */
        typename make_message_bags<output_ports>::type output() const {
          typename make_message_bags<output_ports>::type outmb;
            for(int i = 0; (unsigned)i < state.output.size(); i++){
              get_messages<out>(outmb).emplace_back(state.output[i]);
            }  
            return outmb;
        }
    
        /**
        * @brief advance function time until next fetched item or infinity if EOS.
        * @return TIME until next internal event.
        */    
        TIME time_advance() const {
          return (state.next==std::numeric_limits<TIME>::infinity()?state.next:state.next-state.last);
        }
      };

  }
}

#endif // CADMIUM_PDEVS_ISTREAM_HPP