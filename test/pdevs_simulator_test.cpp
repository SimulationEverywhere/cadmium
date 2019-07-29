/**
 * Copyright (c) 2013-2016, Damian Vicino
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
#include <cadmium/logger/tuple_to_ostream.hpp>

#include <cadmium/basic_model/pdevs/accumulator.hpp>
#include <cadmium/basic_model/pdevs/generator.hpp>
#include <cadmium/engine/pdevs_simulator.hpp>



template<typename TIME>
using int_accumulator=cadmium::basic_models::pdevs::accumulator<int, TIME>;
using int_accumulator_defs=cadmium::basic_models::pdevs::accumulator_defs<int>;

BOOST_AUTO_TEST_SUITE( pdevs_simulator_suite )

BOOST_AUTO_TEST_SUITE( pdevs_accumulator_suite )
BOOST_AUTO_TEST_CASE( accumulator_model_simulation_test )
{
    //construct a simulator for an accumulator
    using simulator_t=cadmium::engine::simulator<int_accumulator, float, cadmium::logger::not_logger>;
    simulator_t s;
    s.init(0.0f);

    BOOST_CHECK(s.next()==std::numeric_limits<float>::infinity());

    using input_ports=int_accumulator<float>::input_ports;
    //crate the tuple for sending the messages
    typename cadmium::make_message_bags<input_ports>::type input_bags=cadmium::make_message_bags<input_ports>::type{};
    typename cadmium::make_message_bags<input_ports>::type empty_input=cadmium::make_message_bags<input_ports>::type{};
    //insert values to add port and reset to empty
    BOOST_REQUIRE_MESSAGE(cadmium::engine::all_bags_empty(input_bags), "Error initializing the bags used for the test");
    cadmium::get_messages<int_accumulator_defs::add>(input_bags).assign(std::initializer_list<int>{1, 2, 3, 4});
    cadmium::get_messages<int_accumulator_defs::reset>(input_bags).clear();

    //advance simulator
    s.inbox(input_bags);
    s.advance_simulation(3.0f);
    BOOST_CHECK(s.next() == std::numeric_limits<float>::infinity());

    //external input in reset triggers a reset
    cadmium::get_messages<int_accumulator_defs::add>(input_bags).clear();
    cadmium::get_messages<int_accumulator_defs::reset>(input_bags).emplace_back();
    s.inbox(input_bags);
    s.advance_simulation(4.0f); //here time is referring to absolute chronology, we are in simulation context.
    BOOST_CHECK(s.next() == 4.0f );

    //out provides the accumulated result
    s.collect_outputs(4.0f);
    auto o = s.outbox();
    BOOST_REQUIRE(!cadmium::engine::all_bags_empty(o));
    BOOST_REQUIRE(cadmium::get_messages<int_accumulator_defs::sum>(o).size() == 1);
    BOOST_CHECK(cadmium::get_messages<int_accumulator_defs::sum>(o).at(0) == 10);
    s.inbox(empty_input);
    s.advance_simulation(4.0f);
    BOOST_CHECK(s.next() == std::numeric_limits<float>::infinity());

    //internal transition resets counter
    s.inbox(input_bags);
    s.advance_simulation(5.0f);
    BOOST_CHECK(s.next() == 5.0f);
    s.collect_outputs(5.0f);
    o = s.outbox();
    BOOST_REQUIRE(!cadmium::engine::all_bags_empty(o));
    BOOST_REQUIRE(cadmium::get_messages<int_accumulator_defs::sum>(o).size() == 1);
    BOOST_CHECK(cadmium::get_messages<int_accumulator_defs::sum>(o).at(0) == 0);
    s.inbox(empty_input);
    s.advance_simulation(5.0f);
    BOOST_CHECK(s.next() == std::numeric_limits<float>::infinity());

    //simultaneous external input in both ports increments and schedules reset
    cadmium::get_messages<int_accumulator_defs::add>(input_bags).assign(std::initializer_list<int>{1, 2, 3, 4});
    s.inbox(input_bags);
    s.advance_simulation(6.0f);
    BOOST_CHECK(s.next() == 6.0f);
    s.collect_outputs(6.0f);
    o = s.outbox();
    BOOST_REQUIRE(!cadmium::engine::all_bags_empty(o));
    BOOST_REQUIRE(cadmium::get_messages<int_accumulator_defs::sum>(o).size() == 1);
    BOOST_CHECK(cadmium::get_messages<int_accumulator_defs::sum>(o).at(0) == 10);
    s.inbox(empty_input);
    s.advance_simulation(6.0f);
    BOOST_CHECK(s.next() == std::numeric_limits<float>::infinity());
}

BOOST_AUTO_TEST_CASE( accumulator_simulation_throws_test ){
    //construct a simulator for an accumulator
    using simulator_t= cadmium::engine::simulator<int_accumulator, float, cadmium::logger::not_logger>;
    using input_ports=int_accumulator<float>::input_ports;
    simulator_t  s;
    s.init(0.0f);
    BOOST_CHECK(s.next()==std::numeric_limits<float>::infinity());

    //crate the tuple for sending the messages
    typename cadmium::make_message_bags<input_ports>::type input_bags=cadmium::make_message_bags<input_ports>::type{};
    typename cadmium::make_message_bags<input_ports>::type empty_input=cadmium::make_message_bags<input_ports>::type{};
    //insert values to add port and reset to empty
    cadmium::get_messages<int_accumulator_defs::add>(input_bags).assign(std::initializer_list<int>{1, 2, 3, 4});
    cadmium::get_messages<int_accumulator_defs::reset>(input_bags).emplace_back();

    //assuming strong exceptions guarantees
    //advance simulator
    s.inbox(input_bags);
    s.advance_simulation(3.0f);
    BOOST_CHECK(s.next() == 3.0f);

    //try to input in the past of current time
    s.inbox(input_bags);
    BOOST_CHECK_THROW(s.advance_simulation(2.0f), std::domain_error);
    //try to input later than next scheduled internal event
    s.inbox(input_bags);
    BOOST_CHECK_THROW(s.advance_simulation(4.0f), std::domain_error);

    //execute expected internal transition
    s.inbox(empty_input);
    s.advance_simulation(3.0f);
    BOOST_CHECK(s.next()==std::numeric_limits<float>::infinity());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( pdevs_generator_suite )

const float init_period = 1.0f;
const float init_output_message = 2.0f;
template<typename TIME>
using floating_generator_base=cadmium::basic_models::pdevs::generator<float, TIME>;
using floating_generator_defs=cadmium::basic_models::pdevs::generator_defs<float>;
template<typename TIME>
struct floating_generator : public floating_generator_base<TIME> {
    float period() const override {
        return init_period;
    }
    float output_message() const override {
        return init_output_message;
    }
};


BOOST_AUTO_TEST_CASE( generator_model_simulation_test )
{
    //construct a simulator for an generator of floats
    using simulator_t=cadmium::engine::simulator<floating_generator, float, cadmium::logger::not_logger>;
    simulator_t s;
    s.init(0.0f);
    BOOST_CHECK(s.next()==1.0f);
    //collecting early output produces a "false output".
    s.collect_outputs(0.5f);
    auto out = s.outbox();
    BOOST_REQUIRE(cadmium::engine::all_bags_empty(out)); // obtaining an empty bag of messages
    //collecting output
    s.collect_outputs(1.0f);
    out = s.outbox();

    BOOST_CHECK(cadmium::get_messages<floating_generator_defs::out>(out).size() == 1);
    BOOST_CHECK(cadmium::get_messages<floating_generator_defs::out>(out)[0] == 2.0f);
    //advance simulation
    s.advance_simulation(1.0f);
    //check next time is 2.0f
    BOOST_CHECK(s.next()==2.0f);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

