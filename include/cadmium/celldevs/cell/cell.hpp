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

#ifndef CADMIUM_CELLDEVS_CELL_HPP
#define CADMIUM_CELLDEVS_CELL_HPP

#include <string>
#include <limits>
#include <utility>
#include <vector>
#include <exception>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/celldevs/utils/utils.hpp>
#include <cadmium/celldevs/cell/msg.hpp>
#include <cadmium/celldevs/delay_buffer/delay_buffer_factory.hpp>


namespace cadmium::celldevs {

    /**
    * Basic input/output port structure for cells.
    * @tparam C the type used for representing a cell ID.
    * @tparam S the type used for representing a cell state.
    */
    template <typename C, typename S>
    struct cell_ports_def{
        struct [[maybe_unused]] cell_in: public cadmium::in_port<cell_state_message<C, S>> {};
        struct [[maybe_unused]] cell_out : public cadmium::out_port<cell_state_message<C, S>> {};
    };

    /**
     * Abstract DEVS atomic model for defining cells in extended Cell-DEVS scenarios.
     * @tparam T the type used for representing time in a simulation.
     * @tparam C the type used for representing a cell ID.
     * @tparam S the type used for representing a cell state.
     * @tparam V the type used for representing a neighboring cell's vicinities. By default, it is set to integer.
     */
    template <typename T, typename C, typename S, typename V=int>
    class cell {
    public:

        using input_ports = std::tuple<typename cell_ports_def<C, S>::cell_in>;
        using output_ports = std::tuple<typename cell_ports_def<C, S>::cell_out>;

        C cell_id;                                          /// Cell ID
        std::vector<C> neighbors;                           /// Neighboring cells' IDs
        T simulation_clock;                                 /// Simulation clock (i.e. current time during a simulation)
        T next_internal;                                    /// Time remaining until next internal state transition
        std::unique_ptr<delay_buffer<T, S>> buffer;         /// output message buffer

        struct state_type {
            S current_state;                                /// Cell's internal state
            std::unordered_map<C, V> neighbors_vicinity;    /// Neighboring cell' vicinities type
            std::unordered_map<C, S> neighbors_state;       /// neighboring cell' public state
        };
        state_type state;

        /// A default constructor is required for compiling issues. However, it is not valid and always throws exception
        cell(){ throw std::invalid_argument("Not enough arguments for initializing a cell"); }

        virtual ~cell() = default;

        /**
         * Creates a new cell with neighbors which vicinities is explicitly specified.
         * @tparam Args additional arguments for initializing the delay buffer
         * @param id ID of the cell to be created.
         * @param neighborhood unordered map which key is a neighboring cell and its value corresponds to the vicinities
         * @param initial_state initial state of the cell.
         * @param output_delay name of the output delay buffer.
         * @param args additional arguments for initializing the output delay buffer.
         */
        template<typename ... Args>
        cell(C const &id, std::unordered_map<C, V> const &neighborhood,
             S initial_state, std::string const &output_delay, Args&&... args) {
            cell_id = id;
            simulation_clock = T();
            next_internal = T();
            state.current_state = initial_state;
            for (auto const &entry: neighborhood) {
                neighbors.push_back(entry.first);
                state.neighbors_vicinity[entry.first] = entry.second;
                state.neighbors_state[entry.first] = S();
            }
            buffer = delay_buffer_factory<T, S>::create_delay_buffer(output_delay, std::forward<Args>(args)...);
            buffer->add_to_buffer(initial_state, T());  // At t = 0, every cell communicates its state to its neighbors
        }

        /**
         * Creates a new cell with neighbors which vicinities is explicitly specified.
         * @param id ID of the cell to be created.
         * @param neighbors vector containing neighboring cells. The vicinities is set to its default value.
         * @param output_delay name of the output delay buffer.
         * @param args additional arguments for initializing the output delay buffer.
         */
        template<typename ... Args>
        cell(C const &id, std::vector<C> const &neighbors, S initial_state,
                std::string const &output_delay, Args&&... args) {
            std::unordered_map<C, V> neighborhood = std::unordered_map<C, V>();
            for (auto const &neighbor: neighborhood)
                neighborhood[neighbor] = V();
            new (this) cell(id, neighborhood, initial_state, output_delay, std::forward<Args>(args)...);
        }

        /************** USER-DEFINED METHODS **************/
        /// @return cell's next state.
        virtual S local_computation() const { return state.current_state; }
        /// @return delay to be applied before communicating to neighbors a new state.
        virtual T output_delay(S const &cell_state) const { return std::numeric_limits<T>::infinity(); }

        /****************** PDEVS METHODS ******************/
        /// internal transition function clears output delay_buffer buffer and updates clock and next time advance.
        void internal_transition() {
            buffer->pop_buffer();
            simulation_clock += time_advance();
            next_internal = buffer->next_timeout() - simulation_clock;
        }

        /**
         * External transition function.
         * It updates clock and next internal event. Then, it refreshes neighbors' state and computes next cell state.
         * if the new cell state is different to the current state, it adds the new state to the output delay buffer.
         * @param e elapsed time from the last event.
         * @param mbs message bag containing new neighbors' state messages.
         */
        void external_transition(T e, typename cadmium::make_message_bags<input_ports>::type mbs) {
            // Update clock and next internal event
            simulation_clock += e;
            next_internal -= e;
            // Refresh the neighbors' current state
            std::vector<cell_state_message<C, S>> bagPortIn = cadmium::get_messages<typename cell_ports_def<C, S>::cell_in>(mbs);
            for (cell_state_message<C, S> msg: bagPortIn) {
                auto it = state.neighbors_state.find(msg.cell_id);
                if (it != state.neighbors_state.end()) {
                    state.neighbors_state[msg.cell_id] = msg.state;
                }
            }
            // Compute next state
            S next = local_computation();
            // If next state is not the current state, then I change my state and schedule my next internal transition
            if (next != state.current_state) {
                buffer->add_to_buffer(next, simulation_clock + output_delay(next));
                next_internal = buffer->next_timeout() - simulation_clock;
            }
            // State is always overwritten (it may not be different enough to send it but different after all)
            state.current_state = next;
        }

        /// Confluence transition function.
        void confluence_transition([[maybe_unused]] T e, typename cadmium::make_message_bags<input_ports>::type mbs) {
            internal_transition();
            external_transition(T(), move(mbs));
        }

        /// Time advance function.
        T time_advance() const { return next_internal; }

        /// @return the next message to be transmitted from the output delay_buffer buffer
        typename cadmium::make_message_bags<output_ports>::type output() const {
            std::vector<cell_state_message<C, S>> bag_port_out = std::vector<cell_state_message<C, S>>();
            bag_port_out.push_back(cell_state_message<C, S>(cell_id, buffer->next_state()));

            typename cadmium::make_message_bags<output_ports>::type bag;
            cadmium::get_messages<typename cell_ports_def<C, S>::cell_out>(bag) = bag_port_out;
            return bag;
        }

        /**
         * Operator overloading function for printing the cell's state.
         * @param os output string stream.
         * @param i cell state.
         * @return output string stream containing the cell state.
         */
        friend std::ostringstream &operator << (std::ostringstream &os, const typename cell<T, C, S, V>::state_type &i) {
            os << i.current_state;
            return os;
        }
    };
} //namespace cadmium::celldevs

#endif //CADMIUM_CELLDEVS_CELL_HPP
