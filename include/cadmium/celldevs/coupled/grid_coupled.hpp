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
        using cells_coupled<T, cell_position, S, V, seq_hash<cell_position>>::add_cell_neighborhood;

        /**
         * Constructor method
         * @param id ID of the Coupled DEVS model that contains the Cell-DEVS scenario
         */
        explicit grid_coupled(std::string const &id) : cells_coupled<T, cell_position, S, V, seq_hash<cell_position>>(id) {}

        /**
         * Adds a lattice of cells to the coupled model
         * @tparam CELL_MODEL model type of the cell to be included
         * @tparam Args any additional parameter required for initializing the cell model
         * @param scenario grid-based Cell-DEVS scenario containing all the information of the lattice
         * @param delay_id ID identifying the type of output delay_buffer buffer used by the cell.
         * @param args any additional parameter required for initializing the cell model
         */
        template <template <typename> typename CELL_MODEL, typename... Args>
        void add_lattice(grid_scenario<S, V> &scenario, std::string const &delay_id, Args&&... args) {
            for (auto const &cell: scenario.get_states()) {
                cell_position cell_id = cell.first;
                cell_map<S, V> map = scenario.get_cell_map(cell_id);
                add_cell<CELL_MODEL, Args...>(map, delay_id, std::forward<Args>(args)...);
            }
        }

        template <template <typename> typename CELL_MODEL, typename... Args>
        void add_cell(cell_map<S, V> &map, std::string const &delayer_id, Args&&... args) {
            cell_position cell_id = map.location;
            S initial_state = map.state;
            cell_unordered<V> neighborhood = map.vicinity;
            add_cell<CELL_MODEL>(cell_id, neighborhood, initial_state, map, delayer_id, std::forward<Args>(args)...);
        }

        virtual void add_grid_cell_json(std::string const &cell_type, cell_map<S, V> &map, std::string const &delay_id,
                                   nlohmann::json const &config) {}

        void add_lattice_json(std::string const &file_in) {
            // Obtain JSON object from file
            std::ifstream i(file_in);
            nlohmann::json j;
            i >> j;
            // Read scenario configuration (default values)
            nlohmann::json s = j["scenario"];
            auto default_cell_type = s["default_cell_type"].get<std::string>();
            auto default_state = (s.contains("default_state"))? s["default_state"].get<S>() : S();
            auto default_vicinity = (s.contains("default_vicinity"))? s["default_vicinity"].get<V>() : V();
            auto default_delay = s["default_delay"].get<std::string>();

            auto shape = s["shape"].get<cell_position>();
            auto wrapped = s["wrapped"].get<bool>();
            grid_scenario<S, V> scenario = grid_scenario<S, V>(shape, default_state, wrapped);

            for (nlohmann::json &n: s["neighborhood"]) {
                auto neighborhood_type = n["type"].get<std::string>();
                auto v = (n.contains("vicinity")) ? n["vicinity"].get<V>() : default_vicinity;
                if (neighborhood_type == "custom") {
                    cell_unordered<V> neighborhood = cell_unordered<V>();
                    for (nlohmann::json &relative: n["neighbors"]) {
                        auto neighbor = relative.get<cell_position>();
                        neighborhood[neighbor] = v;
                    }
                    scenario.add_neighborhood(neighborhood);
                } else {
                    auto range = (n.contains("range")) ? n["range"].get<int>() : 1;
                    if (neighborhood_type == "von_neumann") {
                        scenario.add_neighborhood(grid_scenario<S, V>::von_neumann_neighborhood(shape.size(), range), v);
                    } else if (neighborhood_type == "moore") {
                        scenario.add_neighborhood(grid_scenario<S, V>::moore_neighborhood(shape.size(), range), v);
                    } else {
                        throw std::bad_typeid();
                    }
                }
            }
            for (auto const &cell: scenario.get_states()) {
                cell_position cell_id = cell.first;
                cell_map<S, V> map = scenario.get_cell_map(cell_id);

                std::string delay_id = default_delay;
                std::string cell_type = default_cell_type;
                auto cell_config = nlohmann::json();
                bool special = false;
                for (nlohmann::json &c: j["cells"]) {
                    auto c_id = c["cell_id"].get<cell_position>();
                    if (cell_id == c_id) {
                        special = true;
                        if (c.contains("state"))
                            map.state = c["state"].get<S>();
                        if (c.contains("cell_type"))
                            cell_type = c["cell_type"].get<std::string>();
                        if (c.contains("delay"))
                            delay_id = c["delay"].get<std::string>();
                        if (c.contains("config"))
                            cell_config = c["config"];
                        else if (s.contains("default_config") && s["default_config"].contains(cell_type))
                            cell_config = s["default_config"][cell_type];
                        break;
                    }
                }
                if (!special && s.contains("default_config") && s["default_config"].contains(cell_type))
                    cell_config = s["default_config"][cell_type];
                add_grid_cell_json(cell_type, map, delay_id, cell_config);
            }
        }
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_GRID_COUPLED_HPP
