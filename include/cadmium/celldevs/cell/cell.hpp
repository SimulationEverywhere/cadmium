/**
* Copyright (c) 2020, Román Cárdenas Rodríguez
* ARSLab - Carleton University
* GreenLSI - Polytechnic University of Madrid
* All rights reserved.
*
* Abstract DEVS atomic model for defining cells in Cell-DEVS scenarios
*/

#ifndef CADMIUM_CELLDEVS_ABSTRACT_CELL_HPP
#define CADMIUM_CELLDEVS_ABSTRACT_CELL_HPP

#include <exception>
#include <iostream>
#include <string>
#include <limits>
#include <algorithm>
#include <utility>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/celldevs/delayer/delayer_factory.hpp>


namespace cadmium::celldevs {
    /**
     * Structure that defines cell messages.
     * @tparam C the used for representing a cell ID.
     * @tparam S the type used for representing a cell state.
     */
    template <typename C, typename S>
    struct cell_state_message {
        C cell_id;
        S state;

        /**
         * @brief Cell state message constructor
         * @param cell_id_in ID of the cell that generates the message
         * @param state_in State to be transmitted by the cell
         */
        explicit cell_state_message(C cell_id_in, S state_in): cell_id(cell_id_in), state(state_in) {}

        /**
         * Operator overloading function for printing cell state messages
         * @tparam C the used for representing a cell ID.
         * @tparam S the type used for representing a cell state.
         * @param os Operating System's string stream
         * @param msg cell state message
         * @return Operating System's string stream containing the stringified cell state
         */
        friend std::ostream &operator << (std::ostream &os, const cell_state_message<C, S> &msg) {
            os << msg.cell_id << " ; " << msg.state;
            return os;
        }
    };


    /**
     * @brief input/output port structure for cells.
     * @tparam C the used for representing a cell ID.
     * @tparam S the type used for representing a cell state.
     */
    template <typename C, typename S>
    struct cell_ports_def{
        struct cell_in: public cadmium::in_port<cell_state_message<C, S>> {};
        struct cell_out : public cadmium::out_port<cell_state_message<C, S>> {};
    };

    /**
     * @brief Abstract DEVS atomic model for defining cells in Cell-DEVS scenarios.
     * @tparam T the type used for representing time in a simulation.
     * @tparam C the used for representing a cell ID.
     * @tparam S the type used for representing a cell state.
     * @tparam V the type used for representing a neighboring cell's vicinity. By default, it is set to integer.
     * @tparam C_HASH the hash function used for creating unordered maps with cell IDs as keys.
     *                By default, it uses the one defined in the standard library
     */
    template <typename T, typename C, typename S, typename V=int, typename C_HASH=std::hash<C>>
    class cell {
    public:

        using input_ports = std::tuple<typename cell_ports_def<C, S>::cell_in>;
        using output_ports = std::tuple<typename cell_ports_def<C, S>::cell_out>;

        using NV = std::unordered_map<C, V, C_HASH>;
        using NS = std::unordered_map<C, S, C_HASH>;


        C cell_id;                          /// Cell ID
        std::vector<C> neighbors;           /// Neighboring cells' IDs
        T clock;                            /// simulation clock
        T next_internal;                    /// Time remaining until next internal state transition
        delayer<T, S> *buffer;              /// output message buffer

        struct state_type {
            S current_state;                /// Cell's internal state
            NV neighbors_vicinity;          /// Neighboring cell' vicinity type
            NS neighbors_state;             /// neighboring cell' public state
        };
        state_type state;

        /// A default constructor is required for compiling issues. However, it is not valid and always throws exception
        cell(){ throw std::exception(); }

        /**
         * @brief Creates a new cell with neighbors which vicinity is set to the default.
         * @tparam Args type of any additional parameter required for creating the output delayer.
         * @param cell_id ID of the cell to be created.
         * @param initial_state initial state of the cell.
         * @param neighbors_in vector containing the ID of the neighboring cells.
         * @param delayer_id ID of the output delayer used by the cell.
         * @param args output delayer buffer additional initialization parameters.
         */
        template <typename... Args>
        cell(C const &cell_id, S initial_state, std::vector<C> const &neighbors_in, std::string const &delayer_id, Args&&... args) {
            NV vicinity = NV();
            for (auto const &neighbor: neighbors_in) {
                vicinity.insert({neighbor, V()});
            }
            new (this) cell(cell_id, initial_state, vicinity, delayer_id, std::forward<Args>(args)...);
        }

        /**
         * @brief Creates a new cell with neighbors which vicinity is explicitly specified.
         * @tparam Args type of any additional parameter required for creating the output delayer.
         * @param cell_id ID of the cell to be created.
         * @param initial_state initial state of the cell.
         * @param vicinity unordered map which key is a neighboring cell and its value corresponds to the vicinity
         * @param delayer_id ID of the output delayer buffer used by the cell.
         * @param args output delayer buffer additional initialization parameters.
         */
        template <typename... Args>
        cell(C const &cell_id_in, S initial_state, NV const &vicinity, std::string const &delayer_id, Args&&... args) {
            assert(std::numeric_limits<T>::has_infinity && "This time base does not define infinity");
            cell_id = cell_id_in;
            clock = T();
            next_internal = T();
            state.current_state = initial_state;
            for (auto const &entry: vicinity) {
                neighbors.push_back(entry.first);
                state.neighbors_vicinity[entry.first] = entry.second;
                state.neighbors_state[entry.first] = S();
            }
            buffer = delayer_factory<T, S>::create_delayer(delayer_id, std::forward<Args>(args)...);
            buffer->add_to_buffer(initial_state, T());  // At t = 0, every cell communicates its state to its neighbors
        }

        /************** USER-DEFINED METHODS **************/
        /// @return cell's next state.
        virtual S local_computation() const { return state.current_state; }
        /// @return delay to be applied before communicating to neighbors a new state.
        virtual T output_delay(S const &cell_state) const { return std::numeric_limits<T>::infinity(); }

        /****************** DEVS METHODS ******************/
        /// @brief the internal transition function clears output delayer buffer and updates clock and next time advance.
        void internal_transition() {
            buffer->pop_buffer();
            clock += time_advance();
            next_internal = buffer->next_timeout() - clock;
        }

        /**
         * External transition function.
         * @brief It updates clock and next internal event. Then, it refreshes neighbors' state and computes next cell state.
         *        if the new cell state is different to the current state, it adds the new state to the output delayer buffer.
         * @param e elapsed time from the last event.
         * @param mbs message bag containing new neighbors' state messages.
         */
        void external_transition(T e, typename cadmium::make_message_bags<input_ports>::type mbs) {
            // Update clock and next internal event
            clock += e;
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
                buffer->add_to_buffer(next, clock + output_delay(next));
                next_internal = buffer->next_timeout() - clock;
            }
            state.current_state = next;
        }

        /// Confluence transition function.
        void confluence_transition(T e, typename cadmium::make_message_bags<input_ports>::type mbs) {
            internal_transition();
            external_transition(T(), move(mbs));
        }

        /// Time advance function.
        T time_advance() const { return next_internal; }

        /// @return the next message to be transmitted from the output delayer buffer
        typename cadmium::make_message_bags<output_ports>::type output() const {
            typename cadmium::make_message_bags<output_ports>::type bag;
            S public_state = buffer->next_state();
            std::vector<cell_state_message<C, S>> bag_port_out = {cell_state_message<C, S>(cell_id, public_state)};
            cadmium::get_messages<typename cell_ports_def<C, S>::cell_out>(bag) = bag_port_out;
            return bag;
        }

        /**
         * Operator overloading function for printing the cell's state
         * @param os Operating System's string stream
         * @param i cell state
         * @return Operating System's string stream containing the stringified cell state
         */
        friend std::ostringstream& operator << (std::ostringstream& os,
                const typename cell<T, C, S, V, C_HASH>::state_type& i) {
            os << i.current_state;
            return os;
        }
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_ABSTRACT_CELL_HPP
