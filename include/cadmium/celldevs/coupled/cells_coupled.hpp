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
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/celldevs/utils/utils.hpp>
#include <cadmium/celldevs/cell/cell.hpp>
#include <cadmium/json/json.hpp>


namespace cadmium::celldevs {
    /**
     * Multi-Agent coupled Cell-DEVS model
     * @tparam T the type used for representing time in a simulation.
     * @tparam C the type used for representing a cell ID.
     * @tparam S the type used for representing a cell state.
     * @tparam V the type used for representing a neighboring cell's vicinities. By default, it is set to integer.
     */
    template<typename T, typename C, typename S, typename V=int>
    class cells_coupled: public cadmium::dynamic::modeling::coupled<T> {
    public:

        using cadmium::dynamic::modeling::coupled<T>::_models;

    protected:

        /**
         * Internal method for adding vicinities to its protected attribute. Users must not call to this method.
         * @param cell_id ID of the cell. It must not be already defined in the coupled model
         * @param vicinities unordered map {neighboring cell ID: vicinities kind}
         */
        void add_cell_neighborhood(C const &cell_id, std::unordered_map<C, V> const &neighborhood) {
            auto neighbors = std::vector<C>();
            for (auto neighbor: neighborhood)
                neighbors.push_back(neighbor.first);
            add_cell_neighborhood(cell_id, neighbors);
        }

        std::unordered_map<C, std::vector<C>> neighborhoods;  /// Unordered map: {cell ID: [Neighbor cell 1, ....]}

        /**
         * Internal method for adding vicinities to its private attribute. Users must not call to this method.
         * @param cell_id ID of the cell. It must not be already defined in the coupled model
         * @param vicinities unordered map {neighboring cell ID: vicinities kind}
         */
        void add_cell_neighborhood(C const &cell_id, std::vector<C> const &neighbors) {
            auto it = neighborhoods.find(cell_id);
            if (it != neighborhoods.end()) {
                throw std::bad_typeid();
            }
            neighborhoods.insert({cell_id, neighbors});
        }

    public:
        /**
         * Constructor of the cells_coupled class
         * @param id ID of the Coupled DEVS model that contains the Cell-DEVS scenario
         */
        explicit cells_coupled(std::string const &id) : cadmium::dynamic::modeling::coupled<T>(id), neighborhoods() {}

        /**
         * Adds a single Cell-DEVS cell to the coupled model
         * @tparam CELL_MODEL model type of the cell to be included
         * @tparam Args any additional parameter required for initializing the cell model
         * @param cell_id ID of the cell. It must not be already defined in the coupled model
         * @param neighborhood unordered map {neighboring cell ID: vicinities kind}
         * @param args any additional parameter required for initializing the cell model
         */
        template <template <typename> typename CELL_MODEL, typename... Args>
        void add_cell(C const &cell_id, std::unordered_map<C, V> const &neighborhood, Args&&... args) {
            add_cell_neighborhood(cell_id, neighborhood);
            _models.push_back(cadmium::dynamic::translate::make_dynamic_atomic_model<CELL_MODEL, T>(
                    get_cell_name(cell_id), cell_id, neighborhood, std::forward<Args>(args)...));
        }

        /**
         * Adds a single Cell-DEVS cell to the coupled model. Vicinity is set to its default value
         * @tparam CELL_MODEL model type of the cell to be included
         * @tparam Args any additional parameter required for initializing the cell model
         * @param cell_id ID of the cell. It must not be already defined in the coupled model
         * @param neighbors vector containing all the IDs of neighboring cells
         * @param args any additional parameter required for initializing the cell model
         */
        template <template <typename> typename CELL_MODEL, typename... Args>
        [[maybe_unused]] void add_cell(C const &cell_id, std::vector<C> const &neighbors, Args&&... args) {
            add_cell_neighborhood(cell_id, neighbors);
            _models.push_back(cadmium::dynamic::translate::make_dynamic_atomic_model<CELL_MODEL, T>(
                    get_cell_name(cell_id), cell_id, neighbors, std::forward<Args>(args)...));
        }

        virtual void add_cell_json(std::string const &cell_type, C const &cell_id,
                                   std::unordered_map<C, V> const &neighborhood, S initial_state,
                                   std::string const &delay_id, cadmium::json const &config) {}

        /**
         * Creates a Cell-DEVS scenario from a JSON file.
         * @param file_in path to the JSON file that describes the scenario.
         */
        void add_cells_json(std::string const &file_in) {
            // Obtain JSON object from file
            std::ifstream i(file_in);
            cadmium::json j;
            i >> j;
            cadmium::json cells = j["cells"];
            // Read default cell
            cadmium::json default_cell = cells["default"];  // TODO check that it exists and all parameters are defined
            auto default_delay = default_cell["delay"].get<std::string>();
            auto default_cell_type = default_cell["cell_type"].get<std::string>();
            auto default_state = (default_cell.contains("state")) ? default_cell["state"].get<S>() : S();
            auto default_config = (default_cell.contains("config")) ? default_cell["config"] : cadmium::json();
            auto default_neighborhood = (default_cell.contains("neighborhood")) ? default_cell["neighborhood"].get<std::unordered_map<C, V>>() : std::unordered_map<C, V>();
            // Read each cell's particular configuration
            for (auto& el: j["cells"].items()) {
                const auto& cell_id = el.key();
                auto cell_conf = el.value();
                if (cell_id == "default") {
                    continue;
                }
                auto delay = (cell_conf.contains("delay")) ? cell_conf["delay"].get<std::string>() : default_delay;
                auto cell_type = (cell_conf.contains("cell_type")) ? cell_conf["cell_type"].get<std::string>() : default_cell_type;
                auto state = (cell_conf.contains("state"))? cell_conf["state"].get<S>() : default_state;
                auto config = (cell_conf.contains("config")) ? cell_conf["config"] : default_config;
                auto neighborhood = (cell_conf.contains("neighborhood")) ? cell_conf["neighborhood"].get<std::unordered_map<C, V>>() : default_neighborhood;
                add_cell_json(cell_type, cell_id, neighborhood, state, delay, config);
            }
        }

        /// The user must call this method right after having included all the cells of the scenario
        void couple_cells() {
            for (auto const &neighborhood: neighborhoods) {
                C cell_to = neighborhood.first;
                std::vector<C> neighbors = neighborhood.second;
                for (auto const &cell_from: neighborhood.second) {
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
