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
        using cell_config_map = std::unordered_map<std::string, cell_config<C, S, V>>;

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

        std::unordered_map<C, std::vector<C>> neighborhoods;    /// Unordered map: {cell ID: [Neighbor cell 1, ....]}
        cadmium::json default_config_json;                      /// JSON chunk with default configuration

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
        explicit cells_coupled(std::string const &id) : cadmium::dynamic::modeling::coupled<T>(id), neighborhoods(), default_config_json() {}

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
            default_config_json = j["cells"]["default"];
            // Read default cell
            auto cell_configs = get_default_configs(j["cells"]);
            for (auto &cell: cell_configs) {
                const auto &cell_id = cell.first;
                if (cell_id == "default") {
                    continue;
                }
                auto cell_conf = cell.second;
                add_cell_json(cell_conf.cell_type, cell_id, cell_conf.neighborhood, cell_conf.state, cell_conf.delay, cell_conf.config);
            }
        }

        cell_config_map get_default_configs(const cadmium::json &cells_config) {
            auto default_configs = cell_config_map({{"default", read_default_cell_config(cells_config["default"])}});
            for (auto &el: cells_config.items()) {
                const auto &config_id = el.key();
                auto config_val = el.value();
                if (config_id == "default") {
                    continue;
                }
                default_configs[config_id] = read_cell_config(config_val, default_configs.at("default"));
            }
            return default_configs;
        }

        cell_config<C, S, V> read_default_cell_config(cadmium::json const &description) {
            auto delay = (description.contains("delay")) ? description["delay"].get<std::string>() : "inertial";
            auto cell_type = (description.contains("cell_type")) ? description["cell_type"].get<std::string>() : "default";
            auto state = (description.contains("state")) ? description["state"].get<S>() : S();
            auto neighborhood = (description.contains("neighborhood"))
                                ? parse_neighborhood(description["neighborhood"]) : std::unordered_map<C, V>();
            auto config = (description.contains("config")) ? description["config"] : cadmium::json();
            return cell_config<C, S, V>(delay, cell_type, state, neighborhood, config);
        }

        cell_config<C, S, V> read_cell_config(cadmium::json const &description, cell_config<C, S, V> const &default_config) {
            auto delay = (description.contains("delay")) ? description["delay"].get<std::string>() : default_config.delay;
            auto cell_type = (description.contains("cell_type")) ? description["cell_type"].get<std::string>()
                                                                 : default_config.cell_type;
            auto state = default_config.state;
            if (description.contains("state")) {
                if (default_config_json.contains("state")) {  // If state is an object and a patch exists, we try to apply it
                    state = patch_default_item<S>(default_config_json["state"], description["state"]);
                } else {
                    state = description["state"].get<S>();
                }
            }
            auto neighborhood = (description.contains("neighborhood"))
                                ? parse_neighborhood(description["neighborhood"]) : default_config.neighborhood;
            auto config = default_config.config;
            if (description.contains("config")) {  // If a patch exists, we try to apply it
                config = cadmium::json::parse(config.dump());
                config.merge_patch(description["config"]);
            }
            return cell_config<C, S, V>(delay, cell_type, state, neighborhood, config);
        }

        virtual std::unordered_map<C, V> parse_neighborhood(const cadmium::json &j) {
            return j.get<std::unordered_map<C, V>>();
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

         template <typename X>
         X patch_default_item(cadmium::json const &d, cadmium::json const &p) {
            auto d_copy = cadmium::json::parse(d.dump());
            d_copy.merge_patch(p);
            return d_copy.get<X>();
        }
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_CELLS_COUPLED_HPP
