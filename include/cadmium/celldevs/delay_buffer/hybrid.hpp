/**
 * Copyright (c) 2020, Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
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

#ifndef CADMIUM_CELLDEVS_HYBRID_DELAY_BUFFER_HPP
#define CADMIUM_CELLDEVS_HYBRID_DELAY_BUFFER_HPP

#include <limits>
#include <utility>
#include <deque>
#include <cadmium/celldevs//delay_buffer/delay_buffer.hpp>

namespace cadmium::celldevs {
    /**
     * Cell-DEVS hybrid output delay buffer.
     * @tparam T the type used for representing time in a simulation.
     * @tparam S the type used for representing a cell state.
     * @see delay_buffer/delay_buffer.hpp
     */
    template <typename T, typename S>
    class hybrid_delay_buffer : public delay_buffer<T, S> {
        S last_state;                                 /// Latest transmitted state
        std::deque<std::pair<T, S>> delayed_outputs;  /// Double-ended queue with pairs <time, state>
    public:
        hybrid_delay_buffer() : delay_buffer<T, S>(), last_state(), delayed_outputs() {}

        /// Pushes new state to the queue
        void add_to_buffer(S state, T scheduled_time) override {
            // We remove all the events scheduled after the new state change to ensure that the new element is the last
            while (!delayed_outputs.empty() && delayed_outputs.back().first >= scheduled_time)
                delayed_outputs.pop_back();
            // We add the new event at the end of the queue
            delayed_outputs.push_back({scheduled_time, state});
        }

        /// If queue is empty, returns infinity. Otherwise, returns the scheduled time of the top element
        T next_timeout() const override {
            return (delayed_outputs.empty())? std::numeric_limits<T>::infinity() : delayed_outputs.front().first;
        }

        /// Returns next state to be scheduled. If queue is empty, it returns latest transmitted state
        S next_state() const override {
            return (delayed_outputs.empty())? last_state : delayed_outputs.front().second;
        }

        /// Removes top element of the priority queue
        void pop_buffer() override {
            if (!delayed_outputs.empty()) {
                last_state = delayed_outputs.front().second;
                delayed_outputs.pop_front();
            }
        }
    };
} //namespace cadmium::celldevs

#endif //CADMIUM_CELLDEVS_HYBRID_DELAY_BUFFER_HPP
