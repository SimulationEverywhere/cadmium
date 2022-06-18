/**
 * Copyright (c) 2019, Ben Earle
 * Carleton University
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

#ifndef CADMIUM_OESTREAM_HPP
#define CADMIUM_OESTREAM_HPP

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <limits>

#include <stddef.h>

#include <string>
#include <fstream>


using namespace std;

using namespace cadmium;

/**
 * @brief output_event_stream PDEVS Model.
 *
 * oestream_output is a simple model that prints the values passed to its
 * input port to a file.
 *
*/

template<typename MSG, typename TIME, typename PORT_TYPE>
class oestream_output {
    using defs=PORT_TYPE;

public:
    // default constructor
    oestream_output() noexcept { }
    oestream_output(const char* file_path) noexcept {
        state.path = file_path;
        state.file.open(state.path);
        state.currentTime = TIME("00:00:00");
    }

    // state definition
    struct state_type{
        const char* path;
        ofstream file;
        bool prop;
        TIME currentTime;
    }; 
    state_type state;

    // ports definition
    using input_ports=std::tuple<typename defs::in>;
    using output_ports=std::tuple<>;

    // internal transition
    void internal_transition() {
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        state.currentTime += e;
        for(const auto &x : get_messages<typename defs::in>(mbs)){
            state.file << state.currentTime << " " << x << "\n";
        }
    }

    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
    }

    // output function
    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
        return bags;
    }

    // time_advance function
    TIME time_advance() const {
        return std::numeric_limits<TIME>::infinity();
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename oestream_output<MSG,TIME,PORT_TYPE>::state_type& i) {
        os << "";
        return os;
    }


};

#endif // CADMIUM_OESTREAM_HPP
