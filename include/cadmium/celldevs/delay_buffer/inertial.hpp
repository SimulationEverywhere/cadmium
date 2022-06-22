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

#ifndef CADMIUM_CELLDEVS_INERTIAL_DELAY_BUFFER_HPP
#define CADMIUM_CELLDEVS_INERTIAL_DELAY_BUFFER_HPP

#include <limits>
#include <cadmium/celldevs//delay_buffer/delay_buffer.hpp>


namespace cadmium::celldevs {
    /**
     * @brief Cell-DEVS inertial output delay_buffer.
     * @tparam T the type used for representing time in a simulation.
     * @tparam S the type used for representing a cell state.
     * @see delay_buffer/delay_buffer.hpp
     */
    template <typename T, typename S>
    class inertial_delay_buffer : public delay_buffer<T, S> {
        S last_state;  /// Latest state to be transmitted
        T time;  /// Time when the state is to be transmitted (i.e., simulation clock + propagation delay)
    public:
        inertial_delay_buffer() : delay_buffer<T, S>(), last_state(), time(std::numeric_limits<T>::infinity()) {}

        /// Changes stored state and scheduled time
        void add_to_buffer(S state, T scheduled_time) override {
            last_state = state;
            time = scheduled_time;
        }

        /// Returns simulation time at which the state is to be transmitted
        T next_timeout() const override { return time; }

        /// Returns state to be transmitted
        S next_state() const override { return last_state; }

        /// Sets the next scheduled time to infinity
        void pop_buffer() override { time = std::numeric_limits<T>::infinity(); }
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_INERTIAL_DELAY_BUFFER_HPP
