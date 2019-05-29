/**
 * Copyright (c) 2017, Cristina Ruiz Martin, Laouen Mayal Louan Belloli
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

#ifndef CADMIUM_IESTREAM_HPP
#define CADMIUM_IESTREAM_HPP

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <limits>

#include <stddef.h>

#include <string>
#include <fstream>


using namespace std;

using namespace cadmium;

/**
 * @brief input_event_stream PDEVS Model.
 *
 * iestream PDEVS Model plays a history of events received by an input stream.
 * The list of events allows to be used as a connector to external tools.
 * The input format is "time output and a custom parser can be defined."
 * Data type MSG must have the operator >> in order to work. Data type TIME must also have the operator >> in order to work.
 * Each line must be an MSG. Therefore the operator >> cannot read inputs that are specified in multiple lines
 *
*/


template<class TIME, class INPUT>
class Parser {
private:
  std::ifstream file;

public:
  // Constructors

  Parser() {}

  Parser(const char* file_path) {
    this->open_file(file_path);
  }
  
  void open_file(const char* file_path) {
    file.open(file_path);
  }

  std::pair<TIME,INPUT> next_timed_input() {
    INPUT result;
    TIME next_time;
    if (file.eof()) throw std::exception();
    file >> next_time;
    file >> result;
    return std::make_pair(next_time,result);
  }

};

template<typename MSG>
struct iestream_input_defs{
    //custom ports
    struct out : public out_port<MSG> {
    };
};

template<typename MSG, typename TIME>
class iestream_input {
    using defs=iestream_input_defs<MSG>;

public:

    // default constructor
    iestream_input() noexcept {
    }
    iestream_input(const char* file_path) noexcept {
        state._parser.open_file(file_path);
    }

    // state definition
    struct state_type{
        Parser<TIME, MSG> _parser;
        MSG _last_input_read;
        vector<MSG> _next_input;
        TIME _simulation_time = TIME();
        TIME _next_time = TIME();
        TIME _next_time2 = TIME();
        bool _initialization = true;
    }; 

    //The state._parser.open_file(parth_to_file) must be done in the model instantiation constructor
    state_type state;

    // ports definition
    using input_ports=std::tuple<>;
    using output_ports=std::tuple<typename defs::out>;

    // internal transition
    void internal_transition() {
        //out << "INTERNAL. IESTREAM " << endl;
        state._simulation_time += state._next_time;
        state._next_input.clear();
        if(state._initialization){
            state._initialization = false;
            try {
                std::pair<TIME, MSG> parsed_line = state._parser.next_timed_input();
                state._next_time = parsed_line.first - state._simulation_time;
                if (state._next_time < TIME({0})) throw std::exception();
                state._last_input_read = parsed_line.second;
                state._next_input.push_back(state._last_input_read);
            } catch(std::exception& e) {
                state._next_time = std::numeric_limits<TIME>::infinity();
            }
            try {
                std::pair<TIME, MSG> parsed_line = state._parser.next_timed_input();
                state._next_time2 = parsed_line.first - state._simulation_time;
                if (state._next_time2 < TIME({0})) throw std::exception();
                state._last_input_read = parsed_line.second;
            } catch(std::exception& e) {
                state._next_time2 = std::numeric_limits<TIME>::infinity();
            }
        }else{
            state._next_time = state._next_time2 - state._next_time;
            state._next_input.push_back(state._last_input_read);
            try {
                std::pair<TIME, MSG> parsed_line = state._parser.next_timed_input();
                state._next_time2 = parsed_line.first - state._simulation_time;
                if (state._next_time2 < TIME({0})) throw std::exception();
                state._last_input_read = parsed_line.second;
            } catch(std::exception& e) {
                state._next_time2 = std::numeric_limits<TIME>::infinity();
            }
        }
        while(state._next_time == state._next_time2 & state._next_time != std::numeric_limits<TIME>::infinity()){
            state._next_input.push_back(state._last_input_read);
            try {
                std::pair<TIME, MSG> parsed_line = state._parser.next_timed_input();
                state._next_time2 = parsed_line.first - state._simulation_time;
                if (state._next_time2 < TIME({0})) throw std::exception();
                state._last_input_read = parsed_line.second;
            } catch(std::exception& e) {
                state._next_time2 = std::numeric_limits<TIME>::infinity();
            }
        }
    }

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
        //cout << "OUTPUT. IESTREAM: ";
        typename make_message_bags<output_ports>::type bags;
        for(int i =0; i<state._next_input.size(); i++){
            cadmium::get_messages<typename defs::out>(bags).emplace_back(state._next_input[i]);
            //cout << state._next_input[i] << " | ";
        }
        //cout << endl;
        return bags;
    }

    // time_advance function
    TIME time_advance() const {
        //cout << "time advance iestream " << state._next_time << endl; 
        return state._next_time;
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename iestream_input<MSG,TIME>::state_type& i) {
        os << "next time: " << i._next_time;
        return os;
    }


};

#endif // CADMIUM_IESTREAM_HPP
