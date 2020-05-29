/**
* Copyright (c) 2020, Román Cárdenas Rodríguez
* ARSLab - Carleton University
* GreenLSI - Polytechnic University of Madrid
* All rights reserved.
*
* Cell-DEVS grid utils: bunch of useful definitions for implementing grid-based Cell-DEVS in Cadmium
**/

#ifndef CADMIUM_CELLDEVS_GRID_UTILS_HPP
#define CADMIUM_CELLDEVS_GRID_UTILS_HPP

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <cassert>
#include <exception>
#include <cmath>
#include <boost/functional/hash.hpp>


// Hash function for enabling sequences to be  keys of unordered maps
template <typename SEQ>
struct seq_hash {
    std::size_t operator() (SEQ const &seq) const {
        return boost::hash_range(seq.begin(), seq.end());
    }
};

// For printing vectors (i.e., cell position)
template <typename X>
std::ostream &operator <<(std::ostream &os, std::vector<X> const &v) {
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
/***************************************************/
/*************** Type definitions ******************/
/***************************************************/
    using cell_position = std::vector<int>;

    template<typename X>
    using cell_unordered = std::unordered_map<cell_position, X, seq_hash<cell_position>>;


/***************************************************/
/******************* Grid utils ********************/
/***************************************************/
    template<typename S, typename V=int>
    class cell_map {
    public:
        cell_position shape;                // Shape of the scenario
        cell_position location;             // Position of the cell
        S state;                            // Initial state of the cell
        cell_unordered<V> vicinity;         // Indicates the vicinity type of neighbor cells
        bool wrapped{};                     // It indicates whether the scenario is wrapped or not

        cell_map() { throw std::exception(); }

        cell_map(cell_position const &shape_in, cell_position const &location_in,
                 S const &state_in, cell_unordered<V> const &vicinity_in, bool wrapped_in);

        int manhattan_distance(cell_position const &a);

        int chebyshev_distance(cell_position const &a);

        double n_norm_distance(cell_position const &a, unsigned int n);

        double euclidean_distance(cell_position const &a);

        cell_position neighbor(cell_position const &relative);

        cell_position relative(cell_position const &neighbor);
    };

// Helper public class for doing operations regarding the cell space of the scenario.
    template<typename S, typename V>
    class grid_scenario {
    private:
        unsigned int dimension;             // Dimension of the grid of the scenario
        cell_position shape;                // Shape of the scenario
        cell_unordered<S> states;           // Initial state of cells in the grid
        cell_unordered<V> vicinity;         // Identity map containing relative position of neighbors
        bool wrapped;                       // It indicates whether the scenario is wrapped or not
    public:
        /********************* GETTERS *********************/
        unsigned int get_dimension() { return dimension; }

        cell_position get_shape() { return shape; }

        cell_unordered<S> get_states() const { return states; }

        cell_unordered<V> get_vicinity() { return vicinity; }

        bool get_wrapped() { return wrapped; }

        /********************* SETTERS *********************/
        void set_initial_state(S state) {
            states = cell_unordered<S>();
            cell_position current = cell_position();
            for (int i = 0; i < dimension; i++)
                current.push_back(0);
            while (true) {
                try {
                    set_initial_state(current, state);
                    current = next_cell(current, 0);
                } catch (std::overflow_error &e) {
                    break;
                }
            }
        }

        void set_initial_state(cell_position const &cell, S state) {
            assert(cell_in_scenario(cell));
            states[cell] = state;
        }

        void set_neighborhood(cell_unordered<V> const &neighborhood_in) {
            vicinity.clear();
            add_neighborhood(neighborhood_in);
        }

        void set_moore_neighborhood(unsigned int range) { set_moore_neighborhood(range, V()); }

        void set_von_neumann_neighborhood(unsigned int range) { set_von_neumann_neighborhood(range, V()); }

        void set_moore_neighborhood(unsigned int range, V const &vicinity_in) {
            vicinity.clear();
            add_neighborhood(moore_neighborhood(dimension, range), vicinity_in);
        }

        void set_von_neumann_neighborhood(unsigned int range, V vicinity_in) {
            vicinity.clear();
            add_neighborhood(von_neumann_neighborhood(dimension, range), vicinity_in);
        }

        void add_neighborhood(std::vector<cell_position> const &neighbors, V const &vicinity_in) {
            add_neighborhood(neighbors_to_vicinity(neighbors, vicinity_in));
        }

        void add_neighborhood(cell_unordered<V> const &vicinity_in) {
            for (auto const &v: vicinity_in) {
                assert(v.first.size() == dimension);
                vicinity.insert_or_assign(v.first, v.second);
            }
        }

        void set_wrapped(bool wrapped_in) {
            wrapped = wrapped_in;
        }
        /***************************************************/
        /***************** CONSTRUCTORS ********************/
        /***************************************************/
        grid_scenario(cell_position const &shape_in, S initial_state, bool wrapped_in) {
            new(this) grid_scenario(shape_in, initial_state, cell_unordered<V>(), wrapped_in);
        }

        grid_scenario(cell_position const &shape_in, S initial_state,
                      std::vector<cell_position> const &neighbors, bool wrapped_in) {
            V v = V();
            cell_unordered<V> vicinity_in = neighbors_to_vicinity(neighbors, v);
            new(this) grid_scenario(shape_in, initial_state, vicinity_in, wrapped_in);
        }

        grid_scenario(cell_position const &shape_in, S initial_state,
                      const cell_unordered<V> vicinity_in, bool wrapped_in) {
            // Assert that the shape of the scenario is well-defined
            for (auto const &d: shape_in)
                assert(d > 0);
            dimension = shape_in.size();
            shape = shape_in;
            set_initial_state(initial_state);
            for (auto const &neighbor: vicinity_in) {
                assert(neighbor.first.size() == dimension);
            }
            vicinity = vicinity_in;
            // Set the wrapped flag
            wrapped = wrapped_in;
        }
        /***************************************************/
        /***************** STATIC METHODS ******************/
        /***************************************************/

        /*************** distance functions ****************/
        // Auxiliary function for obtaining the distance vector between two cell
        static cell_position distance_vector(cell_position const &origin, cell_position const &destination,
                                             cell_position const &shape, bool wrapped) {
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
        static cell_position destination_cell(cell_position const &origin, cell_position const &distance,
                                              cell_position const &shape, bool wrapped) {
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
        static int manhattan_distance(cell_position const &a, cell_position const &b,
                                      cell_position const &shape, bool wrapped) {
            int res = 0;
            for (auto const &d: distance_vector(a, b, shape, wrapped))
                res += std::abs(d);
            return res;
        }

        // Auxiliary function for obtaining the Chebyshev distance between two cell of the grid
        static int chebyshev_distance(cell_position const &a, cell_position const &b,
                                      cell_position const &shape, bool wrapped) {
            int res = 0;
            for (auto const &d: distance_vector(a, b, shape, wrapped)) {
                auto d_abs = std::abs(d);
                res = (d_abs > res) ? d_abs : res;
            }
            return res;
        }

        // Auxiliary function for obtaining the N-norm distance between two cell of the grid
        static double n_norm_distance(cell_position const &a, cell_position const &b, unsigned int n,
                                      cell_position const &shape, bool wrapped) {
            assert(n > 0);
            double res = 0;
            for (auto const &d: distance_vector(a, b, shape, wrapped))
                res += std::pow((float) std::abs(d), n);
            return std::pow(res, 1.0 / n);
        }

        // Auxiliary function for obtaining the Euclidean distance between two cell of the grid
        static double euclidean_distance(cell_position const &a, cell_position const &b,
                                         cell_position const &shape, bool wrapped) {
            return n_norm_distance(a, b, 2, shape, wrapped);
        }

        /************* Neighborhoods functions *************/
        static cell_unordered<V>
        neighbors_to_vicinity(std::vector<cell_position> const &neighbors, V const &vicinity_in) {
            cell_unordered<V> res = cell_unordered<V>();
            for (auto const &neighbor: neighbors) {
                res[neighbor] = vicinity_in;
            }
            return res;
        }

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
        cell_position distance_vector(cell_position const &origin, cell_position const &destination) {
            return distance_vector(origin, destination, shape, wrapped);
        }

        cell_position destination_cell(cell_position const &origin, cell_position const &distance) {
            return destination_cell(origin, distance, shape, wrapped);
        }

        int manhattan_distance(cell_position const &a, cell_position const &b) {
            return manhattan_distance(a, b, shape, wrapped);
        }

        int chebyshev_distance(cell_position const &a, cell_position const &b) {
            return chebyshev_distance(a, b, shape, wrapped);
        }

        double n_norm_distance(cell_position const &a, cell_position const &b, unsigned int n) {
            return n_norm_distance(a, b, n, shape, wrapped);
        }

        double euclidean_distance(cell_position const &a, cell_position const &b) {
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
            cell_unordered<V> v = cell_unordered<V>();
            for (auto const &neighbor: vicinity) {
                cell_position relative = neighbor.first;
                V kind = neighbor.second;
                try {
                    cell_position absolute = destination_cell(cell, relative);
                    v.insert({absolute, kind});
                } catch (std::overflow_error &e) {  // Only if neighbor is valid will it be added to the map
                    continue;
                }
            }
            return cell_map<S, V>(shape, cell, states[cell], v, wrapped);
        }
    };

    template<typename S, typename V>
    cell_map<S, V>::cell_map(cell_position const &shape_in, cell_position const &location_in,
                             S const &state_in, cell_unordered<V> const &vicinity_in, bool wrapped_in) {
        shape = shape_in;
        location = location_in;
        state = state_in;
        vicinity = vicinity_in;
        wrapped = wrapped_in;
    }

    template<typename S, typename V>
    int cell_map<S, V>::manhattan_distance(cell_position const &a) {
        return grid_scenario<S, V>::manhattan_distance(location, a, shape, wrapped);
    }

    template<typename S, typename V>
    int cell_map<S, V>::chebyshev_distance(cell_position const &a) {
        return grid_scenario<S, V>::chebyshev_distance(location, a, shape, wrapped);
    }

    template<typename S, typename V>
    double cell_map<S, V>::n_norm_distance(cell_position const &a, unsigned int n) {
        return grid_scenario<S, V>::n_norm_distance(location, a, n, shape, wrapped);
    }

    template<typename S, typename V>
    double cell_map<S, V>::euclidean_distance(cell_position const &a) {
        return n_norm_distance(a, 2);
    }

    template<typename S, typename V>
    cell_position cell_map<S, V>::neighbor(const cell_position &relative) {
        return grid_scenario<S, V>::destination_cell(location, relative, shape, wrapped);
    }

    template<typename S, typename V>
    cell_position cell_map<S, V>::relative(const cell_position &neighbor) {
        return grid_scenario<S, V>::distance_vector(location, neighbor, shape, wrapped);
    }

} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_GRID_UTILS_HPP
