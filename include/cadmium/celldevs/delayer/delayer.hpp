/**
* Copyright (c) 2020, Román Cárdenas Rodríguez
* ARSLab - Carleton University
* GreenLSI - Polytechnic University of Madrid
* All rights reserved.
*
* Interface for implementing Cell-DEVS output delayers.
*/

#ifndef CADMIUM_CELLDEVS_DELAYER_HPP
#define CADMIUM_CELLDEVS_DELAYER_HPP

#include <limits>


namespace cadmium::celldevs {
    /**
     * @brief interface for implementing Cell-DEVS output delayers.
     * @tparam T the type used for representing time in a simulation.
     * @tparam S the type used for representing a cell state.
     */
    template <typename T, typename S>
    class delayer {
    public:
        /**
         * @brief adds a new state to the output buffer, and schedules its propagation at a given time.
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
#endif //CADMIUM_CELLDEVS_DELAYER_HPP
