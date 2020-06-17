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

#ifndef CADMIUM_CELLDEVS_GRID_COUPLED_HPP
#define CADMIUM_CELLDEVS_GRID_COUPLED_HPP

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/celldevs/coupled/cells_coupled.hpp>
#include <cadmium/celldevs/cell/grid_cell.hpp>
#include <cadmium/celldevs/utils/grid_utils.hpp>


namespace cadmium::celldevs {
    /**
     * Coupled Cell-DEVS model for Lattice-based scenarios
     * @tparam T the type used for representing time in a simulation.
     * @tparam S the type used for representing a cell state.
     * @tparam V the type used for representing a neighboring cell's vicinity. By default, it is set to integer.
     * @see coupled/cells_coupled.hpp
     */
    template <typename T, typename S, typename V=int>
    class grid_coupled: public cells_coupled<T, cell_position, S, V, seq_hash<cell_position>> {
    public:

        using cells_coupled<T, cell_position, S, V, seq_hash<cell_position>>::get_cell_name;
        using cells_coupled<T, cell_position, S, V, seq_hash<cell_position>>::add_cell;
        using cells_coupled<T, cell_position, S, V, seq_hash<cell_position>>::add_cell_vicinity;

        /**
         * @brief constructor method
         * @param id ID of the Coupled DEVS model that contains the Cell-DEVS scenario
         */
        explicit grid_coupled(std::string const &id) : cells_coupled<T, cell_position, S, V, seq_hash<cell_position>>(id) {}

        /**
         * @brief adds a lattice of cells to the coupled model
         * @tparam CELL_MODEL model type of the cell to be included
         * @tparam Args any additional parameter required for initializing the cell model
         * @param scenario grid-based Cell-DEVS scenario containing all the information of the lattice
         * @param delayer_id ID identifying the type of output delayer buffer used by the cell.
         * @param args any additional parameter required for initializing the cell model
         */
        template <template <typename> typename CELL_MODEL, typename... Args>
        void add_lattice(std::string const &delayer_id, grid_scenario<S, V> &scenario, Args&&... args) {
            for (auto const &cell: scenario.get_states()) {
                delayer<T, S> *buffer = delayer_factory<T, S>::create_delayer(delayer_id);
                cell_position cell_id = cell.first;
                cell_map<S, V> map = scenario.get_cell_map(cell_id);
                add_cell<CELL_MODEL, Args...>(buffer, map, std::forward<Args>(args)...);
            }
        }

        template <template <typename> typename CELL_MODEL, typename... Args>
        void add_cell(delayer<T, S> *buffer, cell_map<S, V> &map, Args&&... args) {
            cell_position cell_id = map.location;
            S initial_state = map.state;
            cell_unordered<V> vicinity = map.vicinity;
            add_cell<CELL_MODEL>(cell_id, initial_state, vicinity, buffer, map, std::forward<Args>(args)...);
        }

        template <template <typename> typename CELL_MODEL, typename... Args>
        void add_lattice_json(std::string const &file_in, Args&&... args) {
            // Obtain JSON object from file
            std::ifstream i(file_in);
            nlohmann::json j;
            i >> j;
            // Read scenario configuration (default values)
            nlohmann::json s = j["scenario"];
            auto shape = s["shape"].get<cell_position>();
            S default_state = S();
            if (s.contains("default_state"))
                default_state = s["default_state"];
            bool wrapped = false;
            if (s.contains("wrapped"))
                wrapped = s["wrapped"];
            grid_scenario<S, V> scenario = grid_scenario<S, V>(shape, default_state, wrapped);
            std::string default_delayer = "transport";
            if (s.contains("default_delayer"))
                default_delayer = s["default_delayer"];

            V default_vicinity = V();
            if (s.contains("default_vicinity"))
                default_vicinity = s["default_vicinity"];
            for (nlohmann::json &n: s["neighborhood"]) {
                std::string neighborhood_type = n["type"];
                V v = default_vicinity;
                if (n.contains("vicinity")) {
                    v = n["vicinity"];
                }
                if (neighborhood_type == "custom") {
                    cell_unordered<V> vicinities = cell_unordered<V>();
                    for (nlohmann::json &relative: n["neighbors"]) {
                        auto neighbor = relative["neighbor"].get<cell_position>();
                        V neighbor_v = v;
                        if (relative.contains("vicinity"))
                            neighbor_v = relative["vicinity"];
                        vicinities[neighbor] = neighbor_v;
                    }
                    scenario.add_neighborhood(vicinities);
                } else {
                    int range = 1;
                    if (n.contains("range"))
                        range = n["range"];
                    if (neighborhood_type == "von_neumann") {
                        scenario.add_neighborhood(grid_scenario<S, V>::von_neumann_neighborhood(shape.size(), range), v);
                    } else if (neighborhood_type == "moore") {
                        scenario.add_neighborhood(grid_scenario<S, V>::moore_neighborhood(shape.size(), range), v);
                    }
                }
            }
            for (nlohmann::json &c: j["cells"]) {
                auto cell = c["cell_id"].get<cell_position>();
                S initial_state = c["state"];
                scenario.set_initial_state(cell, initial_state);
            }
            for (auto const &cell: scenario.get_states()) {
                cell_position cell_id = cell.first;
                cell_map<S, V> map = scenario.get_cell_map(cell_id);
                std::string delayer_id = default_delayer;
                for (nlohmann::json &c: j["cells"]) {
                    auto c_id = c["cell_id"].get<cell_position>();
                    if (cell_id == c_id && c.contains("delayer")) {
                        delayer_id = c["delayer"];
                        break;
                    }
                }
                delayer<T, S> *buffer = delayer_factory<T, S>::create_delayer(delayer_id);
                add_cell<CELL_MODEL, Args...>(buffer, map, std::forward<Args>(args)...);
            }
        }
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_GRID_COUPLED_HPP
