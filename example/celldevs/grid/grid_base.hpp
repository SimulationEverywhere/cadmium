//
// Created by Román Cárdenas Rodríguez on 26/05/2020.
//

#ifndef CADMIUM_CELLDEVS_GRID_BASE_HPP
#define CADMIUM_CELLDEVS_GRID_BASE_HPP

#include <cadmium/celldevs/cell/grid_cell.hpp>

using namespace cadmium::celldevs;

template <typename T>
class grid_base : public grid_cell<T, int, int> {
public:
    using grid_cell<T, int, int>::cell_id;
    using grid_cell<T, int, int>::state;
    using grid_cell<T, int, int>::map;

    grid_base() : grid_cell<T, int, int>() { }

    template<typename... Args>
    grid_base(cell_map<int, int> const &map_in, std::string const &delayer_id, Args&&... args):
        grid_cell<T, int, int>(map_in, delayer_id, std::forward<Args>(args)...) {}

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
        int delay = 0;
        for (int i: cell_id)
            delay += i;
        return 4 - delay;
    }
};

#endif //CADMIUM_CELLDEVS_GRID_BASE_HPP
