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

    grid_base(cell_position const &cell_id, int initial_state, cell_unordered<int> const &vicinities,
              delayer<T, int> *buffer, cell_map<int, int> const &map_in):
              grid_cell<T, int, int>(cell_id, initial_state, vicinities, buffer, map_in) {}


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
