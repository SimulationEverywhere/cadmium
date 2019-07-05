/**
 * Copyright (c) 2013-2017, Damian Vicino
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


#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <cadmium/basic_model/pdevs/accumulator.hpp>
#include <cadmium/engine/pdevs_simulator.hpp>
#include <cadmium/engine/pdevs_coordinator.hpp>
#include <cadmium/basic_model/pdevs/generator.hpp>
#include <cadmium/engine/pdevs_engine_helpers.hpp>

/**
  * This test is for some common helper functions used by coordinators and simulators
  */
BOOST_AUTO_TEST_SUITE( pdevs_engine_helpers_test_suite )

////Definition of an accumulator of floats
template<typename TIME>
using floating_accumulator=cadmium::basic_models::pdevs::accumulator<float, TIME>;
using floating_accumulator_defs=cadmium::basic_models::pdevs::accumulator_defs<float>;
//Definition of a simulator
using simulator_of_floating_accumulator=cadmium::engine::simulator<floating_accumulator, float, cadmium::logger::not_logger>;
//Definition of a tuple with one simulator
using tuple_sim_accum=std::tuple<simulator_of_floating_accumulator>;
BOOST_AUTO_TEST_CASE(get_engine_by_model__one_element_test){
    tuple_sim_accum st;
    cadmium::engine::get_engine_by_model<floating_accumulator<float>, tuple_sim_accum>(st);
}

//Definition of two generators
const float init_period = 0.1f;
const float init_output_message = 1.0f;
template<typename TIME>
using floating_generator_base=cadmium::basic_models::pdevs::generator<float, TIME>;
using floating_generator_defs=cadmium::basic_models::pdevs::generator_defs<float>;
template<typename TIME>
struct floating_generator_a : public floating_generator_base<TIME> {
    float period() const override {
        return init_period;
    }
    float output_message() const override {
        return init_output_message;
    }
};

template<typename TIME>
struct floating_generator_b : public floating_generator_base<TIME> {
    float period() const override {
        return init_period;
    }
    float output_message() const override {
        return init_output_message;
    }
};

//Definition of two simulator
using simulator_of_gen_a=cadmium::engine::simulator<floating_generator_a, float, cadmium::logger::not_logger>;
using simulator_of_gen_b=cadmium::engine::simulator<floating_generator_b, float, cadmium::logger::not_logger>;
//Definition of a tuple with one simulator
using tuple_sim_gens=std::tuple<simulator_of_gen_a, simulator_of_gen_b>;
BOOST_AUTO_TEST_CASE(get_engine_by_model_two_elements_get_first_test){
    tuple_sim_gens st;
    auto eng_a=cadmium::engine::get_engine_by_model<floating_generator_a<float>, tuple_sim_gens>(st);
}

BOOST_AUTO_TEST_CASE(get_engine_by_model_two_elements_get_last_test){
    tuple_sim_gens st;
    auto eng_b=cadmium::engine::get_engine_by_model<floating_generator_b<float>, tuple_sim_gens>(st);
}


BOOST_AUTO_TEST_SUITE_END()
