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

#ifndef CADMIUM_CELLDEVS_MSG_HPP
#define CADMIUM_CELLDEVS_MSG_HPP

#include <iostream>
#include <cadmium/modeling/ports.hpp>


/**
 * Auxiliary function for printing vectors
 * @tparam X vector content type
 * @param os output stream
 * @param v vector
 * @return output stream containing the printed values of the vector
 */
template <typename X>
std::ostream &operator << (std::ostream &os, std::vector<X> const &v) {
    os << "(";
    std::string separator;
    for (auto x : v) {
        os << separator << x;
        separator = ",";
    }
    os << ")";
    return os;
}

namespace cadmium::celldevs {
    /**
     * Structure that defines cell messages.
     * @tparam C the type used for representing a cell ID.
     * @tparam S the type used for representing a cell state.
     */
    template <typename C, typename S>
    struct cell_state_message {
        C cell_id;
        S state;

        /**
         * Cell state message constructor
         * @param cell_id_in ID of the cell that generates the message
         * @param state_in State to be transmitted by the cell
         */
        cell_state_message(C cell_id_in, S state_in): cell_id(cell_id_in), state(state_in) {}

        /**
         * Operator overloading function for printing cell state messages
         * @tparam C the type used for representing a cell ID.
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
     * Input/output port structure for cells.
     * @tparam C the type used for representing a cell ID.
     * @tparam S the type used for representing a cell state.
     */
    template <typename C, typename S>
    struct cell_ports_def{
        struct cell_in: public cadmium::in_port<cell_state_message<C, S>> {};
        struct cell_out : public cadmium::out_port<cell_state_message<C, S>> {};
    };

} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_MSG_HPP
