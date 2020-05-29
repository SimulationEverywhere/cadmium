/**
* Copyright (c) 2020, Román Cárdenas Rodríguez
* ARSLab - Carleton University
* GreenLSI - Polytechnic University of Madrid
* All rights reserved.
*
* Cell-DEVS transport output delayer.
*/

#ifndef CADMIUM_CELLDEVS_TRANSPORT_HPP
#define CADMIUM_CELLDEVS_TRANSPORT_HPP

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
        transport_delayer() : delayer<T, S>() {}

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
