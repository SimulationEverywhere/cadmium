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

#ifndef CADMIUM_CELLDEVS_GRID_CELL_HPP
#define CADMIUM_CELLDEVS_GRID_CELL_HPP

#include <cadmium/celldevs/cell/cell.hpp>
#include <cadmium/celldevs/utils/grid_utils.hpp>


namespace cadmium::celldevs {
    /**
     * @brief DEVS atomic model for defining cells in Cell-DEVS scenarios that are sorted in lattices.
     * @tparam T the type used for representing time in a simulation.
     * @tparam S the type used for representing a cell state.
     * @tparam P the type used for representing cell configuration parameters. By default, it is set to integer.
     * @tparam V the type used for representing a neighboring cell's vicinity. By default, it is set to integer.
     */
    template <typename T, typename S, typename V=int>
    class grid_cell : public cell<T, cell_position, S, V, seq_hash<cell_position>> {
    public:

        cell_map<S, V> map;     /// Cell map

        grid_cell() : cell<T, cell_position, S, V, seq_hash<cell_position>>() {}

        /**
         * @brief Creates a new cell that belongs to a lattice of cells.
         * @param location ID of the cell to be created.
         * @param neighborhood unordered map which key is a neighboring cell and its value corresponds to the vicinity
         * @param buffer_in pointer to the output delayer object of the cell.
         * @param initial_state initial state of the cell.
         * @param map_in grid map with a bunch of utilities
         * @see utils/grid_utils.hpp
         */
        grid_cell(cell_position const &location, cell_unordered<V> const &neighborhood, delayer<T, S> *buffer_in,
                S initial_state, cell_map<S, V> const &map_in) :
            cell<T, cell_position, S, V, seq_hash<cell_position>>(location, neighborhood, buffer_in, initial_state),
        map{map_in} {}
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_GRID_CELL_HPP
