//
// Created by Román Cárdenas Rodríguez on 30/04/2020.
//
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "ScenarioShape"

#include <boost/test/unit_test.hpp>
#include <cadmium/celldevs/utils/grid_utils.hpp>

using namespace cadmium::celldevs;


int moore_cells(int dimension, int range) {
    return std::pow(2 * range + 1, dimension);
}

BOOST_AUTO_TEST_CASE(moore) {
    for (unsigned int D = 1; D < 5; D++) {
        for (unsigned int r = 0; r < 4; r++) {
            cell_position shape = cell_position();
            cell_position middle = cell_position();
            for (int d = 0; d < D; d++) {
                shape.push_back(2 * r + 1);
                middle.push_back(r);
            }
            std::vector<cell_position> neighbors = grid_scenario<int, int>::biassed_moore_neighborhood(D, r);
            BOOST_CHECK_EQUAL(neighbors.size(), moore_cells(D, r));
            for (auto const &cell: neighbors) {
                int a = grid_scenario<int, int>::chebyshev_distance(middle, cell, shape, false);
                BOOST_CHECK_LE( a, r);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(von_neumann) {
    for (unsigned int D = 1; D < 5; D++) {
        for (unsigned int r = 0; r < 4; r++) {
            cell_position shape = cell_position();
            cell_position middle = cell_position();
            for (int d = 0; d < D; d++) {
                shape.push_back(2 * r + 1);
                middle.push_back(r);
            }
            std::vector<cell_position> neighbors = grid_scenario<int, int>::biassed_von_neumann_neighborhood(D, r);
            for (auto const &cell: neighbors) {
                int a = grid_scenario<int, int>::manhattan_distance(middle, cell, shape, false);
                BOOST_CHECK_LE(a, r);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(grid_test_2d) {
    cell_position scenario_shape = {10, 10};

    grid_scenario space = grid_scenario<int, int>(scenario_shape, 0, true);
    space.set_initial_state({0, 0}, 1);
    space.set_von_neumann_neighborhood(1);
    cell_unordered<int> neighbors = space.get_vicinity();
    BOOST_CHECK(!space.cell_in_scenario({10, 10}));
    BOOST_CHECK(!space.cell_in_scenario({-1, 0}));
    BOOST_CHECK(space.cell_in_scenario({0, 9}));
    BOOST_CHECK(space.cell_in_scenario({0, 0}));
    cell_position ref = {0, 0};
    cell_map<int, int> neighborhood = space.get_cell_map(ref);
    BOOST_CHECK_EQUAL(neighborhood.vicinity.size(), space.get_vicinity().size());
    for (auto &cell_mapping: neighborhood.vicinity) {
        BOOST_CHECK_LT(space.manhattan_distance(ref, cell_mapping.first), 2);
    }

    space = grid_scenario(scenario_shape, 0, neighbors, false);
    neighborhood = space.get_cell_map(ref);
    BOOST_CHECK_NE(neighborhood.vicinity.size(), space.get_vicinity().size());
    for (auto &cell_mapping: neighborhood.vicinity) {
        BOOST_CHECK_LT(space.manhattan_distance(ref, cell_mapping.first), 2);
    }
    ref = {3, 3};
    cell_map<int, int> cell_u = space.get_cell_map(ref);
}
