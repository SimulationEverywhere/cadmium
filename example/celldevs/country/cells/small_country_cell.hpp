//
// Created by Román Cárdenas Rodríguez on 19/06/2020.
//

#ifndef CADMIUM_SMALL_COUNTRY_CELL_HPP
#define CADMIUM_SMALL_COUNTRY_CELL_HPP

#include <exception>
#include <string>
#include <nlohmann/json.hpp>
#include <cadmium/celldevs/cell/cell.hpp>

using namespace cadmium::celldevs;

using cell_id_t = std::string; using state_t = int;

template <typename TIME>
class small_country_cell: public cadmium::celldevs::cell<TIME, cell_id_t, state_t> {
public:
    using cadmium::celldevs::cell<TIME, cell_id_t, state_t>::cell_id;
    using cadmium::celldevs::cell<TIME, cell_id_t, state_t>::state;

    std::string config = "hola";

    small_country_cell() : cadmium::celldevs::cell<TIME, cell_id_t, state_t>() {}

    small_country_cell(const cell_id_t &cell_id, std::unordered_map<cell_id_t , state_t> const &neighborhood,
                       state_t initial_state, std::string const &delay_id, std::string config_in):
            cadmium::celldevs::cell<TIME, cell_id_t, state_t>(cell_id, neighborhood, initial_state, delay_id),
                    config(config_in) {}

    using config_type = std::string;  // IMPORTANT FOR THE JSON

    // user must define this function. It returns the next cell state and its corresponding timeout
    int local_computation() const override {
        int res = state.current_state;
        for (auto other: state.neighbors_state) {
            res = (other.second > res)? other.second : res;
        }
        return res;
    }
    // It returns the delay to communicate cell's new state.
    TIME output_delay(int const &cell_state) const override { return 1; }
};

#endif //CADMIUM_SMALL_COUNTRY_CELL_HPP
