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

#ifndef CADMIUM_CELLDEVS_CELLS_COUPLED_HPP
#define CADMIUM_CELLDEVS_CELLS_COUPLED_HPP

#include <fstream>
#include <iostream>
#include <exception>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/celldevs/cell/cell.hpp>
#include <cadmium/celldevs/delayer/delayer_factory.hpp>


namespace cadmium::celldevs {
    /**
     * standard coupled Cell-DEVS model
     * @tparam T the type used for representing time in a simulation.
     * @tparam C the type used for representing a cell ID.
     * @tparam S the type used for representing a cell state.
     * @tparam V the type used for representing a neighboring cell's vicinity. By default, it is set to integer.
     * @tparam C_HASH the hash function used for creating unordered maps with cell IDs as keys.
     *                By default, it uses the one defined in the standard library
     */
    template<typename T, typename C, typename S, typename V=int, typename C_HASH=std::hash<C>>
    class cells_coupled: public cadmium::dynamic::modeling::coupled<T> {
    public:

        template <typename X>
        using cell_unordered = std::unordered_map<C, X, C_HASH>;  // alias for unordered maps with cell IDs as key

        using cadmium::dynamic::modeling::coupled<T>::_models;

        cell_unordered<cell_unordered<V>> neighborhoods;  /// Nested unordered map: {cell ID to: {cell ID from: Vicinity between cells}}

        /**
         * @brief constructor method
         * @param id ID of the Coupled DEVS model that contains the Cell-DEVS scenario
         */
        explicit cells_coupled(std::string const &id) : cadmium::dynamic::modeling::coupled<T>(id), neighborhoods() {}

        /**
         * @brief adds a single Cell-DEVS cell to the coupled model
         * @tparam CELL_MODEL model type of the cell to be included
         * @tparam Args any additional parameter required for initializing the cell model
         * @param cell_id ID of the cell. It must not be already defined in the coupled model
         * @param neighborhood unordered map {neighboring cell ID: vicinity kind}
         * @param buffer pointer to output delayer buffer to be used by the cell.
         * @param initial_state Initial state of the cell
         * @param args any additional parameter required for initializing the cell model
         */
        template <template <typename> typename CELL_MODEL, typename... Args>
        void add_cell(C const &cell_id, cell_unordered<V> const &neighborhood, Args&&... args) {
            add_cell_neighborhood(cell_id, neighborhood);
            _models.push_back(cadmium::dynamic::translate::make_dynamic_atomic_model<CELL_MODEL, T>(
                    get_cell_name(cell_id), cell_id, neighborhood, std::forward<Args>(args)...));
        }

        /**
         * adds a single Cell-DEVS cell to the coupled model. Vicinity is set to its default value
         * @tparam CELL_MODEL model type of the cell to be included
         * @tparam Args any additional parameter required for initializing the cell model
         * @param cell_id ID of the cell. It must not be already defined in the coupled model
         * @param neighbors vector containing all the IDs of neighboring cells
         * @param buffer pointer to output delayer buffer to be used by the cell.
         * @param initial_state Initial state of the cell
         * @param args any additional parameter required for initializing the cell model
         */
        template <template <typename> typename CELL_MODEL, typename... Args>
        void add_cell(C const &cell_id, std::vector<C> const &neighbors, Args&&... args) {
            cell_unordered<V> neighborhood = cell_unordered<V>();
            for (auto const &neighbor: neighbors) {
                neighborhood.insert({neighbor, V()});
            }
            add_cell<CELL_MODEL, Args...>(cell_id, neighborhood, std::forward<Args>(args)...);
        }

        /**
         * Creates a Cell-DEVS scenario from a JSON file
         * @tparam CELL_MODEL model type of the cells to be included
         * @tparam Args any additional parameter required for initializing the cell model
         * @param file_in path to the JSON file that describes the scenario
         * @param args any additional parameter required for initializing the cell model
         */
        template <template <typename> class CELL_MODEL>
        void add_cells_json(std::string const &file_in) {
            delayer_factory_registry<T, S> delay_factory = delayer_factory_registry<T,S>();
            // Obtain JSON object from file
            std::ifstream i(file_in);
            nlohmann::json j;
            i >> j;
            // Read scenario configuration (default values)
            nlohmann::json s = j["scenario"];
            S default_state = S();
            if (s.contains("default_state"))
                default_state = s["default_state"].get<S>();
            V default_vicinity = V();
            if (s.contains("default_vicinity"))
                default_vicinity = s["default_vicinity"].get<V>();
            auto default_delayer_id = s["default_delayer"].get<std::string>();
            // Read each cell's particular configuration
            for (nlohmann::json &c: j["cells"]) {
                C cell_id = c["cell_id"];
                S initial_state = default_state;
                if (c.contains("state"))
                    initial_state = c["state"].get<S>();
                std::string delayer_id = default_delayer_id;
                if (c.contains("delayer"))
                    delayer_id = c["delayer"].get<std::string>();
                V v = default_vicinity;
                if (c.contains("default_vicinity"))
                    v = c["default_vicinity"].get<V>();
                cell_unordered<V> vicinity = cell_unordered<V>();
                for (nlohmann::json &n: c["neighborhood"]) {
                    C neighbor_id = n["cell_id"].get<C>();
                    V neighbor_vicinity = v;
                    if (n.contains("vicinity"))
                        neighbor_vicinity = n["vicinity"].get<V>();
                    vicinity[neighbor_id] = neighbor_vicinity;
                }
                delayer<T, S> *buffer = delay_factory.create_delayer(delayer_id);
                if (c.contains("config")) {
                    auto config = c["config"].get<typename CELL_MODEL<T>::config_type>();
                    add_cell<CELL_MODEL>(cell_id, vicinity, buffer, initial_state, config);
                } else if (s.contains("default_config")) {
                    auto config = s["default_config"].get<typename CELL_MODEL<T>::config_type>();
                    add_cell<CELL_MODEL>(cell_id, vicinity, buffer, initial_state, config);
                } else {
                    add_cell<CELL_MODEL>(cell_id, vicinity, buffer, initial_state);
                }
            }
        }

        /// The user must call this method right after having included all the cells of the scenario
        void couple_cells() {
            for (auto const &neighborhood: neighborhoods) {
                C cell_to = neighborhood.first;
                cell_unordered<V> neighbors = neighborhood.second;
                for (auto const &neighbor: neighbors) {
                    C cell_from = neighbor.first;
                    cadmium::dynamic::modeling::coupled<T>::_ic.push_back(
                            cadmium::dynamic::translate::make_IC<
                                    typename cell_ports_def<C, S>::cell_out,
                                    typename cell_ports_def<C, S>::cell_in
                            >(get_cell_name(cell_from), get_cell_name(cell_to))
                    );
                }
            }
        }

        /**
         * @brief internal method for adding vicinities to its private attribute. Users must not call to this method.
         * @param cell_id ID of the cell. It must not be already defined in the coupled model
         * @param vicinities unordered map {neighboring cell ID: vicinity kind}
         */
        void add_cell_neighborhood(C const &cell_id, cell_unordered<V> const &vicinities_in) {
            auto it = neighborhoods.find(cell_id);
            if (it != neighborhoods.end()) {
                throw std::bad_typeid();
            }
            neighborhoods.insert({cell_id, vicinities_in});
        }

        /**
         * @brief returns a "stringified" version of a cell ID.
         * @param cell_id cell ID
         * @return "stringified" version of a cell ID.
         */
        std::string get_cell_name(C const &cell_id) const {
            std::stringstream model_name;
            model_name << cadmium::dynamic::modeling::coupled<T>::get_id() << "_" << cell_id;
            return model_name.str();
        }
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_CELLS_COUPLED_HPP
