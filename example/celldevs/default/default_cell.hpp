//
// Created by Román Cárdenas Rodríguez on 25/05/2020.
//

#ifndef CADMIUM_CELLDEVS_DEFAULT_CELL_HPP
#define CADMIUM_CELLDEVS_DEFAULT_CELL_HPP

#include <exception>
#include <cadmium/celldevs/cell/cell.hpp>

template <typename T>
class default_cell: public cadmium::celldevs::cell<T, int, int> {
public:
    using cadmium::celldevs::cell<T, int, int>::cell_id;
    using cadmium::celldevs::cell<T, int, int>::state;

    explicit default_cell() : cadmium::celldevs::cell<T, int, int>() {}

    template <typename... Args>
    default_cell(int cell_id, int initial_state, std::vector<int> const &neighbors, std::string const &delayer_id, Args&&... args):
            cadmium::celldevs::cell<T, int, int>(cell_id, initial_state, neighbors, delayer_id, std::forward<Args>(args)...) {}

    template <typename... Args>
    default_cell(int cell_id, int initial_state, std::unordered_map<int, int> const &vicinities, std::string const &delayer_id, Args&&... args):
            cadmium::celldevs::cell<T, int, int>(cell_id, initial_state, vicinities, delayer_id, std::forward<Args>(args)...) {}

    // user must define this function. It returns the next cell state and its corresponding timeout
    int local_computation() const override {
        int res = state.current_state;
        for (auto other: state.neighbors_state) {
            res = (other.second > res)? other.second : res;
        }
        return res;
    }
    // It returns the delay to communicate cell's new state.
    T output_delay(int const &cell_state) const override {
        return 3 - cell_id;
    }
};

#endif //CADMIUM_CELLDEVS_DEFAULT_CELL_HPP
