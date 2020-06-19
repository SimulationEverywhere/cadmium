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

#ifndef CADMIUM_COUNTRY_CELL_HPP
#define CADMIUM_COUNTRY_CELL_HPP

#include <exception>
#include <string>
#include <cadmium/celldevs/cell/cell.hpp>
#include <cadmium/celldevs/delayer/delayer.hpp>

using namespace cadmium::celldevs;

using cell_id_t = std::string; using state_t = int;

template <typename TIME>
class country_cell: public cadmium::celldevs::cell<TIME, cell_id_t, state_t> {
public:
    using cadmium::celldevs::cell<TIME, cell_id_t, state_t>::cell_id;
    using cadmium::celldevs::cell<TIME, cell_id_t, state_t>::state;

    using config_type = int;  // IMPORTANT FOR THE JSON
    config_type config;

    explicit country_cell() : cadmium::celldevs::cell<TIME, cell_id_t, state_t>() {}

    country_cell(const cell_id_t &cell_id, std::unordered_map<cell_id_t , state_t> const &vicinities,
            delayer<TIME, state_t> *buffer, state_t initial_state, int config_in):
        cadmium::celldevs::cell<TIME, cell_id_t, state_t>(cell_id, vicinities, buffer, initial_state), config(config_in) {}

    country_cell(const cell_id_t &cell_id, std::unordered_map<cell_id_t , state_t> const &vicinities,
                 delayer<TIME, state_t> *buffer, state_t initial_state) {
        new (this) country_cell(cell_id, vicinities, buffer, initial_state, 0);
    }

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

#endif //CADMIUM_COUNTRY_CELL_HPP
