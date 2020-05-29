/**
* Copyright (c) 2020, Román Cárdenas Rodríguez
* ARSLab - Carleton University
* GreenLSI - Polytechnic University of Madrid
* All rights reserved.
*
* Cell-DEVS inertial output delayer.
*/

#ifndef CADMIUM_CELLDEVS_INERTIAL_DELAYER_HPP
#define CADMIUM_CELLDEVS_INERTIAL_DELAYER_HPP

#include <limits>
#include <cadmium/celldevs/delayer/delayer.hpp>


namespace cadmium::celldevs {
    /**
     * @brief Cell-DEVS inertial output delayer.
     * @tparam T the type used for representing time in a simulation.
     * @tparam S the type used for representing a cell state.
     * @see delayer/delayer.hpp
     */
    template <typename T, typename S>
    class inertial_delayer : public delayer<T, S> {
    private:
        S state_buffer;     /// Latest (and only) state to be transmitted
        T time;             /// Time when the state is to be transmitted (i.e., simulation clock + propagation delay)
    public:
        inertial_delayer() : delayer<T, S>() {
            state_buffer = S();
            time = std::numeric_limits<T>::infinity();
        }

        /// Changes stored buffer and scheduled time
        void add_to_buffer(S state, T scheduled_time) override {
            state_buffer = state;
            time = scheduled_time;
        }

        /// Returns simulation time at which the state is to be transmitted
        T next_timeout() const override { return time; }

        /// Returns state to be transmitted
        S next_state() const override { return state_buffer; }

        /// Sets the next scheduled time to infinity
        void pop_buffer() override { time = std::numeric_limits<T>::infinity(); }
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_INERTIAL_DELAYER_HPP
