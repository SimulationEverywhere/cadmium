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

#ifndef CADMIUM_CELLDEVS_TRANSPORT_HPP
#define CADMIUM_CELLDEVS_TRANSPORT_HPP

#include <iostream>
#include <functional>
#include <limits>
#include <queue>
#include <tuple>
#include <vector>
#include <cadmium/celldevs/delayer/delayer.hpp>


namespace cadmium::celldevs {
    /**
     * @brief Cell-DEVS transport output delayer.
     * @tparam T the type used for representing time in a simulation.
     * @tparam S the type used for representing a cell state.
     * @see delayer/delayer.hpp
     */
    template <typename T, typename S>
    class transport_delayer : public delayer<T, S> {
    private:
        /// Queue with pairs <scheduled time, state to transmit>. Messages are sorted depending on the scheduled time
        std::priority_queue<std::pair<T, S>, std::vector<std::pair<T, S>>, std::greater<std::pair<T, S>>> delayed_outputs;
    public:
        transport_delayer() : delayer<T, S>() { }

        /// Pushes new state to the priority queue
        void add_to_buffer(S state, T scheduled_time) override {
            delayed_outputs.push(std::make_pair(scheduled_time, state));
        }

        /// If queue is empty, returns infinity. Otherwise, returns the scheduled time of the top element
        T next_timeout() const override {
            return (delayed_outputs.empty())? std::numeric_limits<T>::infinity() : delayed_outputs.top().first;
        }

        /// Returns next state to be scheduled. If priority queue is empty, it causes undefined behavior
        S next_state() const override { return delayed_outputs.top().second; }

        /// Removes top element of the priority queue
        void pop_buffer() override { delayed_outputs.pop(); }
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_TRANSPORT_HPP
