/**
* Copyright (c) 2020, Román Cárdenas Rodríguez
* ARSLab - Carleton University
* GreenLSI - Polytechnic University of Madrid
* All rights reserved.
*
* Cells for modeling lattice-based Cell-DEVS scenarios
*/

#ifndef CADMIUM_CELLDEVS_GRID_CELLS_HPP
#define CADMIUM_CELLDEVS_GRID_CELLS_HPP

#include <exception>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/celldevs/cell/cell.hpp>
#include <cadmium/celldevs/utils/grid_utils.hpp>


namespace cadmium::celldevs {
    /**
     * @brief DEVS atomic model for defining cells in Cell-DEVS scenarios that are sorted in lattices.
     * @tparam T the type used for representing time in a simulation.
     * @tparam S the type used for representing a cell state.
     * @tparam V the type used for representing a neighboring cell's vicinity. By default, it is set to integer.
     */
    template <typename T, typename S, typename V=int>
    class grid_cell : public cell<T, cell_position, S, V, seq_hash<cell_position>> {
    public:

        cell_map<S, V> map;     /// Cell map

        grid_cell() : cell<T, cell_position, S, V, seq_hash<cell_position>>() {}

        /**
         * @brief Creates a grid-based cell
         * @tparam Args type of any additional parameter required for creating the output delayer
         * @param map_in auxiliary grid cell map
         * @param delayer_id ID of the output delayer buffer used by the cell
         * @param args output delayer buffer additional initialization parameters
         * @see utils/grid_utils.hpp
         * @see cell/cell.hpp
         */
        template <typename ...Args>
        grid_cell(cell_map<S, V> const &map_in, std::string const &delayer_id, Args &&... args):
                cell<T, cell_position, S, V, seq_hash<cell_position>>(map_in.location, map_in.state,
                        map_in.vicinity, delayer_id, std::forward<Args>(args)...), map{map_in} {}
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_GRID_CELLS_HPP
