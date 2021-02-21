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
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/celldevs/coupled/cells_coupled.hpp>
#include <cadmium/celldevs/cell/grid_cell.hpp>
#include <cadmium/celldevs/utils/utils.hpp>
#include <cadmium/celldevs/utils/grid_utils.hpp>
#include <cadmium/json/json.hpp>


namespace cadmium::celldevs {
    /**
     * Coupled Cell-DEVS model for Lattice-based scenarios
     * @tparam T the type used for representing time in a simulation.
     * @tparam S the type used for representing a cell state.
     * @tparam V the type used for representing a neighboring cell's vicinities. By default, it is set to integer.
     * @see coupled/cells_coupled.hpp
     */
    template <typename T, typename S, typename V=int>
    class grid_coupled: public cells_coupled<T, cell_position, S, V> {
    private:
        cell_position shape = cell_position();
        bool wrapped = false;
    public:
        using cells_coupled<T, cell_position, S, V>::default_config_json;
        using cells_coupled<T, cell_position, S, V>::get_default_configs;
        using cells_coupled<T, cell_position, S, V>::get_cell_name;
        using cells_coupled<T, cell_position, S, V>::add_cell;
        using cells_coupled<T, cell_position, S, V>::add_cell_neighborhood;

        /**
         * Constructor method
         * @param id ID of the Coupled DEVS model that contains the Cell-DEVS scenario
         */
        explicit grid_coupled(std::string const &id) : cells_coupled<T, cell_position, S, V>(id) {}

        /**
         * Adds a lattice of cells to the coupled model
         * @tparam CELL_MODEL model type of the cell to be included
         * @tparam Args any additional parameter required for initializing the cell model
         * @param scenario grid-based Cell-DEVS scenario containing all the information of the lattice
         * @param delay_id ID identifying the type of output delay_buffer buffer used by the cell.
         * @param args any additional parameter required for initializing the cell model
         */
        template<template<typename> typename CELL_MODEL, typename... Args>
        [[maybe_unused]] void add_lattice(grid_scenario<S, V> &scenario, std::string const &delay_id, Args &&... args) {
            for (auto const &cell: scenario.get_states()) {
                cell_position cell_id = cell.first;
                cell_map<S, V> map = scenario.get_cell_map(cell_id);
                add_cell<CELL_MODEL, Args...>(map, delay_id, std::forward<Args>(args)...);
            }
        }

        template<template<typename> typename CELL_MODEL, typename... Args>
        void add_cell(cell_map<S, V> &map, std::string const &delayer_id, Args &&... args) {
            cell_position cell_id = map.location;
            S initial_state = map.state;
            cell_unordered<V> neighborhood = map.neighborhood;
            add_cell<CELL_MODEL>(cell_id, neighborhood, initial_state, map, delayer_id, std::forward<Args>(args)...);
        }

        virtual void add_grid_cell_json(std::string const &cell_type, cell_map<S, V> &map, std::string const &delay_id,
                                        cadmium::json const &config) {}

        void add_lattice_json(std::string const &file_in) {
            // Obtain JSON object from file
            std::ifstream i(file_in);
            cadmium::json j;
            i >> j;
            // read shape and wrapped option
            shape = j["shape"].get<cell_position>();
            wrapped = j.contains("wrapped") && j["wrapped"].get<bool>();
            // Read cell configurations and create default scenario
            default_config_json = j["cells"]["default"];
            auto default_configs = get_default_configs(j["cells"]);
            grid_scenario<S, V> scenario = grid_scenario<S, V>(shape, default_configs.at("default"), wrapped);
            // Set special configurations
            for (auto const &el: j["cell_map"].items()) {
                auto config = default_configs.at(el.key());
                for (auto const &c: el.value()) {
                    auto cell = c.get<cell_position>();
                    scenario.set_initial_config(cell, config);
                }
            }
            // Create cells and add them to the coupled DEVS model
            for (auto const &cell: scenario.configs) {
                auto cell_id = cell.first;
                auto config = cell.second;
                auto map = scenario.get_cell_map(cell_id);
                add_grid_cell_json(config.cell_type, map, config.delay, config.config);
            }
        }

        cell_unordered<V> parse_neighborhood(const cadmium::json &j) override {
            auto neighborhood = cell_unordered< V>();
            for (const cadmium::json &n: j) {
                auto type = n["type"].get<std::string>();
                auto vicinity = n["vicinity"].get<V>();
                if (type == "custom" || type == "relative") {
                    if (type == "custom") {
                        std::cerr << "Deprecation warning: \"custom\" neighborhood type has been changed to \"relative\". Change it in your JSON configuration file.\n";
                    }
                    for (const cadmium::json &relative: n["neighbors"]) {
                        auto neighbor = relative.get<cell_position>();
                        neighborhood[neighbor] = vicinity;
                    }
                } else if (type == "absolute" || type == "remove") {
                    throw std::logic_error("Neighborhood type not yet implemented");  // TODO implement these neighborhood types
                } else {
                    auto range = (n.contains("range")) ? n["range"].get<int>() : 1;
                    std::vector<cell_position> neighbors;
                    if (type == "von_neumann") {
                        neighbors =  grid_scenario<S, V>::von_neumann_neighborhood(shape.size(), range);
                    } else if (type == "moore") {
                        neighbors = grid_scenario<S, V>::moore_neighborhood(shape.size(), range);
                    } else {
                        throw std::bad_typeid();
                    }
                    for (const auto &neighbor: neighbors) {
                        neighborhood[neighbor] = vicinity;
                    }
                }
            }
            return neighborhood;
        }
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_GRID_COUPLED_HPP
