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

#ifndef CADMIUM_CELLDEVS_GRID_UTILS_HPP
#define CADMIUM_CELLDEVS_GRID_UTILS_HPP

#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <cassert>
#include <exception>
#include <cmath>
#include <boost/functional/hash.hpp>
#include <cadmium/celldevs/utils/utils.hpp>


namespace cadmium::celldevs {

    using cell_position = std::vector<int>;  /// Alias to refer to a cell.

    /**
     * Alias to refer to an unordered map with cell positions as keys.
     * @tparam X type of the values stored in the unordered map.
     */
    template<typename X>
    using cell_unordered = std::unordered_map<cell_position, X>;

    /**
     * Cell configuration structure.
     * @tparam S type used to represent cell states.
     * @tparam V type used to represent vicinities between cells.
     */
    template <typename S, typename V>
    using grid_cell_config = cell_config<cell_position, S, V>;

    /**
     * Auxiliary class with useful functions for grid cells.
     * @tparam S type used to represent cell states.
     * @tparam V type used to represent vicinities between cells.
     */
    template<typename S, typename V=int>
    class cell_map {
    public:
        cell_position shape;                /// Shape of the scenario
        cell_position location;             /// Location of the cell
        S state;                            /// Initial state of the cell
        cell_unordered<V> neighborhood;     /// Indicates the vicinities type of neighbor cells
        bool wrapped;                       /// It indicates whether the scenario is wrapped or not

        cell_map() { throw std::exception(); }

        cell_map(cell_position shape, cell_position location, S const &state, cell_unordered<V> const &neighborhood, bool wrapped);

        [[maybe_unused]] [[nodiscard]] int manhattan_distance(cell_position const &a) const;

        [[maybe_unused]] [[nodiscard]] int chebyshev_distance(cell_position const &a) const;

        [[maybe_unused]] [[nodiscard]] double n_norm_distance(cell_position const &a, unsigned int n) const;

        [[maybe_unused]] [[nodiscard]] double euclidean_distance(cell_position const &a) const;

        [[maybe_unused]] [[nodiscard]] cell_position neighbor(cell_position const &relative) const;

        [[maybe_unused]] [[nodiscard]] cell_position relative(cell_position const &neighbor) const;
    };


    /**
     * Class used by grid_coupled to set the scenario.
     * @tparam S type used to represent cell states.
     * @tparam V type used to represent vicinities between cells.
     */
    template<typename S, typename V>
    class grid_scenario {
    public:
        cell_position shape;                             /// Shape of the scenario.
        unsigned int dimension;                          /// Dimension of the grid of the scenario.
        cell_unordered<grid_cell_config<S, V>> configs;  /// Configuration of cells in the grid.
        bool wrapped;                                    /// It indicates whether the scenario is wrapped or not.

        void set_initial_config(const grid_cell_config<S, V> &config) {
            configs = cell_unordered<grid_cell_config<S, V>>();
            cell_position current = cell_position();
            for (int i = 0; i < dimension; i++)
                current.push_back(0);
            while (true) {
                try {
                    set_initial_config(current, config);
                    current = next_cell(current, 0);
                } catch (std::overflow_error &e) {
                    break;
                }
            }
        }

        void set_initial_config(const cell_position &cell, const grid_cell_config<S, V> config) {
            assert(cell_in_scenario(cell));
            configs[cell] = config;
        }

        grid_scenario(const cell_position &shape, const grid_cell_config<S, V> &config, bool wrapped):
        shape(shape), dimension(shape.size()), configs(), wrapped(wrapped) {
            // Assert that the shape of the scenario is well-defined
            set_initial_config(config);
            for (auto const &d: shape)
                assert(d > 0);
            for (auto const &cell: configs) {
                assert(cell.first.size() == dimension);
            }
        }

        /***************************************************/
        /***************** STATIC METHODS ******************/
        /***************************************************/

        /*************** distance functions ****************/
        // Auxiliary function for obtaining the distance vector between two cell
        static cell_position distance_vector(const cell_position &origin, const cell_position &destination,
                                             const cell_position &shape, bool wrapped) {
            assert(cell_in_scenario(origin, shape) && cell_in_scenario(destination, shape));
            cell_position res = cell_position();
            for (int i = 0; i < origin.size(); i++) {
                auto diff = destination[i] - origin[i];
                if (wrapped && std::abs(diff) > shape[i] / 2)
                    diff = (diff < 0) ? diff + shape[i] : diff - shape[i];
                res.push_back(diff);
            }
            return res;
        }

        // Auxiliary function for obtaining the destination cell from the origin cell and the distance vector
        static cell_position destination_cell(const cell_position &origin, const cell_position &distance,
                                              const cell_position &shape, bool wrapped) {
            assert(cell_in_scenario(origin, shape) && distance.size() == shape.size());
            for (int i = 0; i < shape.size(); i++)
                assert(std::abs(distance[i]) < shape[i]);
            cell_position res = cell_position();
            for (int i = 0; i < origin.size(); i++) {
                auto dest = origin[i] + distance[i];
                if (wrapped)
                    dest = (dest + shape[i]) % shape[i];
                res.push_back(dest);
            }
            if (!cell_in_scenario(res, shape))
                throw std::overflow_error("Destination cell is not in scenario");
            return res;
        }

        // Auxiliary function for obtaining the Manhattan distance between two cell of the grid
        [[maybe_unused]] static int manhattan_distance(const cell_position &a, const cell_position &b,
                                                       const cell_position &shape, bool wrapped) {
            int res = 0;
            for (auto const &d: distance_vector(a, b, shape, wrapped))
                res += std::abs(d);
            return res;
        }

        // Auxiliary function for obtaining the Chebyshev distance between two cell of the grid
        [[maybe_unused]] static int chebyshev_distance(const cell_position &a, const cell_position &b,
                                                       const cell_position &shape, bool wrapped) {
            int res = 0;
            for (auto const &d: distance_vector(a, b, shape, wrapped)) {
                auto d_abs = std::abs(d);
                res = (d_abs > res) ? d_abs : res;
            }
            return res;
        }

        // Auxiliary function for obtaining the N-norm distance between two cell of the grid
        [[maybe_unused]] static double n_norm_distance(const cell_position &a, const cell_position &b, unsigned int n,
                                                       const cell_position &shape, bool wrapped) {
            assert(n > 0);
            double res = 0;
            for (auto const &d: distance_vector(a, b, shape, wrapped))
                res += std::pow((float) std::abs(d), n);
            return std::pow(res, 1.0 / n);
        }

        // Auxiliary function for obtaining the Euclidean distance between two cell of the grid
        [[maybe_unused]] static double euclidean_distance(const cell_position &a, const cell_position &b,
                                                          const cell_position &shape, bool wrapped) {
            return n_norm_distance(a, b, 2, shape, wrapped);
        }

        /************* Neighborhoods functions *************/
        // Auxiliary function for generating biassed Moore neighborhoods (i.e., center cell is not (0,0...)
        static std::vector<cell_position> biassed_moore_neighborhood(unsigned int dimension, unsigned int range) {
            std::vector<cell_position> res = std::vector<cell_position>();
            cell_position scenario_shape = cell_position();
            cell_position current = cell_position();
            for (int i = 0; i < dimension; i++) {
                scenario_shape.push_back(2 * range + 1);
                current.push_back(0);
            }
            while (true) {
                res.push_back(current);
                try {
                    current = next_cell(current, scenario_shape, 0);
                }
                catch (std::overflow_error &e) {
                    break;
                }
            }
            return res;
        }

        // Auxiliary function for unbiassing a neighborhood as a function of a middle cell
        static void unbias_neighborhood(std::vector<cell_position> &biassed_neighborhood, const cell_position &middle) {
            for (auto &cell: biassed_neighborhood) {
                int dimension = cell.size();
                for (int i = 0; i < dimension; i++) {
                    cell[i] -= middle[i];
                }
            }
        }

        // Auxiliary function for generating von Neumann neighborhoods
        static std::vector<cell_position> biassed_von_neumann_neighborhood(unsigned int dimension, unsigned int range) {
            std::vector<cell_position> res = std::vector<cell_position>();
            std::vector<cell_position> moore = biassed_moore_neighborhood(dimension, range);
            cell_position middle = cell_position();
            cell_position shape = cell_position();
            for (int i = 0; i < dimension; i++) {
                shape.push_back(2 * range + 1);
                middle.push_back(range);
            }
            for (auto const &neighbor: moore) {
                if (manhattan_distance(middle, neighbor, shape, false) <= range)
                    res.push_back(neighbor);
            }
            return res;
        }

        // Auxiliary function for generating Moore neighborhoods
        static std::vector<cell_position> moore_neighborhood(unsigned int dimension, unsigned int range) {
            std::vector<cell_position> res = biassed_moore_neighborhood(dimension, range);
            cell_position middle = cell_position();
            for (int i = 0; i < dimension; i++) {
                middle.push_back(range);
            }
            unbias_neighborhood(res, middle);
            return res;
        }

        // Auxiliary function for generating von Neumann neighborhoods
        static std::vector<cell_position> von_neumann_neighborhood(unsigned int dimension, unsigned int range) {
            std::vector<cell_position> res = biassed_von_neumann_neighborhood(dimension, range);
            cell_position middle = cell_position();
            for (int i = 0; i < dimension; i++) {
                middle.push_back(range);
            }
            unbias_neighborhood(res, middle);
            return res;
        }
        /*************** Cell space-related **************/
        // Auxiliary function for iterating over cell of a scenario
        static cell_position next_cell(cell_position last_cell, cell_position const &scenario_shape, int d) {
            // If the dimension being explored has not reached the maximum, we just sum 1 to this dimension
            if (last_cell[d] < scenario_shape[d] - 1) {
                last_cell[d]++;
                return last_cell;
                // If the d is not the maximum dimension, we trigger a recursive call for incrementing the next dimension
            } else if (d < last_cell.size() - 1) {
                last_cell[d] = 0;
                return next_cell(last_cell, scenario_shape, d + 1);
                // Otherwise, we have reached the limit of our scenario. Throw overflow error.
            } else {
                throw std::overflow_error("Reached the last cell of the scenario");
            }
        }

        // Returns true if cell is within the boundaries of the scenario
        static bool cell_in_scenario(cell_position const &cell, cell_position const &shape) {
            assert(cell.size() == shape.size());
            for (int i = 0; i < shape.size(); i++) {
                if (cell[i] < 0 || cell[i] >= shape[i])
                    return false;
            }
            return true;
        }

        /***************************************************/
        /*************** NON-STATIC METHODS ****************/
        /***************************************************/

        /*************** Distance functions ****************/
        [[maybe_unused]] cell_position distance_vector(cell_position const &origin, cell_position const &destination) {
            return distance_vector(origin, destination, shape, wrapped);
        }

        [[maybe_unused]] cell_position destination_cell(cell_position const &origin, cell_position const &distance) {
            return destination_cell(origin, distance, shape, wrapped);
        }

        [[maybe_unused]] int manhattan_distance(cell_position const &a, cell_position const &b) {
            return manhattan_distance(a, b, shape, wrapped);
        }

        [[maybe_unused]] int chebyshev_distance(cell_position const &a, cell_position const &b) {
            return chebyshev_distance(a, b, shape, wrapped);
        }

        [[maybe_unused]] double n_norm_distance(cell_position const &a, cell_position const &b, unsigned int n) {
            return n_norm_distance(a, b, n, shape, wrapped);
        }

        [[maybe_unused]] double euclidean_distance(cell_position const &a, cell_position const &b) {
            return n_norm_distance(a, b, 2);
        }
        /*************** Cell space-related **************/
        // Returns true if cell is within the boundaries of the scenario
        bool cell_in_scenario(cell_position const &cell) {
            return cell_in_scenario(cell, shape);
        }

        cell_position next_cell(cell_position last_cell, int d) {
            return next_cell(std::move(last_cell), shape, d);
        }

        // Returns unordered map {relative_neighbor: absolute_neighbor} for a given cell
        cell_map<S, V> get_cell_map(const cell_position &cell) {
            assert(cell_in_scenario(cell));
            auto config = configs[cell];
            cell_unordered<V> neighborhood = cell_unordered<V>();
            for (auto const &neighbor: config.neighborhood) {
                cell_position relative = neighbor.first;
                V vicinity = neighbor.second;
                try {
                    cell_position absolute = destination_cell(cell, relative);
                    neighborhood.insert({absolute, vicinity});
                } catch (std::overflow_error &e) {  // Only if neighbor is valid will it be added to the map
                    continue;
                }
            }
            return cell_map<S, V>(shape, cell, config.state, neighborhood, wrapped);
        }
    };

    template<typename S, typename V>
    cell_map<S, V>::cell_map(cell_position shape, cell_position location, const S &state, const cell_unordered<V> &neighborhood, bool wrapped) :
        shape(std::move(shape)), location(std::move(location)), state(state), neighborhood(neighborhood), wrapped(wrapped) {}

    template<typename S, typename V>
    int cell_map<S, V>::manhattan_distance(const cell_position &a) const {
        return grid_scenario<S, V>::manhattan_distance(location, a, shape, wrapped);
    }

    template<typename S, typename V>
    int cell_map<S, V>::chebyshev_distance(const cell_position &a) const {
        return grid_scenario<S, V>::chebyshev_distance(location, a, shape, wrapped);
    }

    template<typename S, typename V>
    double cell_map<S, V>::n_norm_distance(const cell_position &a, unsigned int n) const {
        return grid_scenario<S, V>::n_norm_distance(location, a, n, shape, wrapped);
    }

    template<typename S, typename V>
    [[maybe_unused]] double cell_map<S, V>::euclidean_distance(const cell_position &a) const {
        return n_norm_distance(a, 2);
    }

    template<typename S, typename V>
    [[maybe_unused]] cell_position cell_map<S, V>::neighbor(const cell_position &relative) const {
        return grid_scenario<S, V>::destination_cell(location, relative, shape, wrapped);
    }

    template<typename S, typename V>
    [[maybe_unused]] cell_position cell_map<S, V>::relative(const cell_position &neighbor) const {
        return grid_scenario<S, V>::distance_vector(location, neighbor, shape, wrapped);
    }
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_GRID_UTILS_HPP
