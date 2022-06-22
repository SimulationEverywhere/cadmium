/**
 * Copyright (c) 2013-2019, Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis
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

//This file will include the clock example

#include <iostream>
#include <chrono>
#include <algorithm>
#include <cadmium/basic_model/pdevs/generator.hpp>
#include <cadmium/modeling/coupling.hpp>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/concept/coupled_model_assert.hpp>
#include <cadmium/engine/pdevs_runner.hpp>

using namespace std;

using hclock=chrono::high_resolution_clock;

/**
 * This example is the simulation of a super simplified clock with 3 needles (H,M,S)
 * Each needle is a generator with a period of 1s, 1m, 1h.
 * The generators are connected to 3 ports (H, M, S)
 *
 * The experiment runtime is measured using the chrono library.
 */


//message representing ticks
struct tick {
};


//generators for tick definition
using out_p = cadmium::basic_models::pdevs::generator_defs<tick>::out;
template<typename TIME>
using tick_generator_base=cadmium::basic_models::pdevs::generator<tick, TIME>;

template<typename TIME>
struct hour_generator : public tick_generator_base<TIME> {
    float period() const override {
        return 3600.0f; //using float for time in this example
    }

    tick output_message() const override {
        return tick();
    }
};

template<typename TIME>
struct minute_generator : public tick_generator_base<TIME> {
    float period() const override {
        return 60.0f; //using float for time in this example
    }

    tick output_message() const override {
        return tick();
    }
};

template<typename TIME>
struct second_generator : public tick_generator_base<TIME> {
    float period() const override {
        return 1.0f; //using float for time in this example
    }

    tick output_message() const override {
        return tick();
    }
};

//clock coupled model definition
using iports = std::tuple<>;
struct H_port : public cadmium::out_port<tick> {
};
struct M_port : public cadmium::out_port<tick> {
};
struct S_port : public cadmium::out_port<tick> {
};
using oports = std::tuple<H_port, M_port, S_port>;
using submodels=cadmium::modeling::models_tuple<hour_generator, minute_generator, second_generator>;

using eics=std::tuple<>;
using eocs=std::tuple<
        cadmium::modeling::EOC<hour_generator, out_p, H_port>,
        cadmium::modeling::EOC<minute_generator, out_p, M_port>,
        cadmium::modeling::EOC<second_generator, out_p, S_port>
>;
using ics=std::tuple<>;

template<typename TIME>
using clock_model=cadmium::modeling::pdevs::coupled_model<TIME, iports, oports, submodels, eics, eocs, ics>;

int main() {
    auto start = hclock::now(); //to measure simulation execution time

    cadmium::engine::runner<float, clock_model> r{0.0};
    r.run_until(30000.0);

    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>
            (hclock::now() - start).count();
    cout << "Simulation took:" << elapsed << "sec" << endl;
    return 0;
}

