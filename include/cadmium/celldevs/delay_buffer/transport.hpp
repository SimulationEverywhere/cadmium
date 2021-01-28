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

#ifndef CADMIUM_CELLDEVS_TRANSPORT_DELAY_BUFFER_HPP
#define CADMIUM_CELLDEVS_TRANSPORT_DELAY_BUFFER_HPP

#include <limits>
#include <queue>
#include <vector>
#include <unordered_map>
#include <cadmium/celldevs//delay_buffer/delay_buffer.hpp>


namespace cadmium::celldevs {
    /**
     * Cell-DEVS transport output delay buffer.
     * @tparam T the type used for representing time in a simulation.
     * @tparam S the type used for representing a cell state.
     * @see delay_buffer/delay_buffer.hpp
     */
    template <typename T, typename S>
    class transport_delay_buffer : public delay_buffer<T, S> {
        S last_state;  /// Latest transmitted state
        std::priority_queue<T, std::vector<T>, std::greater<T>> timeline;  /// Queue with times with scheduled events
        std::unordered_map<T, S> delayed_outputs;  /// Unordered map {scheduled time: state to transmit}
    public:
        transport_delay_buffer() : delay_buffer<T, S>(), last_state(), timeline(), delayed_outputs() {}

        /// Pushes new state to the queue
        void add_to_buffer(S state, T scheduled_time) override {
            // If no previous events are scheduled at the desired time, we add the time to the timeline
            if (delayed_outputs.find(scheduled_time) == delayed_outputs.end())
                timeline.push(scheduled_time);
            // We add the new state to the delayed outputs buffer (and override previous state if collision)
            delayed_outputs.insert_or_assign(scheduled_time, state);
        }

        /// If queue is empty, returns infinity. Otherwise, returns the scheduled time of the top element
        T next_timeout() const override {
            return (timeline.empty())? std::numeric_limits<T>::infinity() : timeline.top();
        }

        /// Returns next state to be scheduled. If priority queue is empty, it returns latest transmitted state
        S next_state() const override {
            return (timeline.empty())? last_state : delayed_outputs.at(timeline.top());
        }

        /// Removes top element of the priority queue
        void pop_buffer() override {
            if (!timeline.empty()) {
                last_state = delayed_outputs.at(timeline.top());  // Store latest valid output in last_state
                delayed_outputs.erase(timeline.top());  // Remove latest valid output from the output buffer...
                timeline.pop();  // .. and from the timeline
            }
        }
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_TRANSPORT_DELAY_BUFFER_HPP
