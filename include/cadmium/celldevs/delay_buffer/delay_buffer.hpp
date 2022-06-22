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

#ifndef CADMIUM_CELLDEVS_DELAY_BUFFER_HPP
#define CADMIUM_CELLDEVS_DELAY_BUFFER_HPP

#include <limits>

namespace cadmium::celldevs {
    /**
     * @brief interface for implementing Cell-DEVS output delay buffers.
     * @tparam T the type used for representing time in a simulation.
     * @tparam S the type used for representing a cell state.
     */
    template <typename T, typename S>
    class delay_buffer {
    public:
        virtual ~delay_buffer() = default;

        /**
         * adds a new state to the output buffer, and schedules its propagation at a given time.
         * @param state state to be transmitted by the cell.
         * @param scheduled_time clock time when this state must be transmitted.
         */
        virtual void add_to_buffer(S state, T scheduled_time) {};

        ///@return clock time for the next scheduled output (i.e., clock + time advance).
        virtual T next_timeout() const { return std::numeric_limits<T>::infinity(); };

        /// @return next cell state to be transmitted.
        virtual S next_state() const { return S(); };

        /// Removes from buffer the next scheduled state transmission.
        virtual void pop_buffer() {};
    };
} //namespace cadmium::celldevs

#endif //CADMIUM_CELLDEVS_DELAY_BUFFER_HPP
