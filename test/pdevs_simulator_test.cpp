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

#include <cadmium/basic_model/accumulator.hpp>
#include <cadmium/engine/pdevs_simulator.hpp>
#include <limits>

template<typename TIME>
using int_accumulator=cadmium::basic_models::accumulator<int, TIME>;

BOOST_AUTO_TEST_SUITE( pdevs_simulator_suite )

BOOST_AUTO_TEST_SUITE( pdevs_accumulator_suite )
BOOST_AUTO_TEST_CASE( accumulator_model_simulation_test )
{
    //construct a simulator for an accumulator
    using simulator_t= cadmium::engine::simulator<int_accumulator, float>;
    using model_t=simulator_t::model_type;
    simulator_t s{0.0f};

    BOOST_CHECK(s.next()==std::numeric_limits<float>::infinity());

    using input_ports=int_accumulator<float>::input_ports;
    //crate the tuple for sending the messages
    typename cadmium::make_message_bags<input_ports>::type input_bags;
    typename cadmium::make_message_bags<input_ports>::type empty_input;
    //insert values to add port and reset to empty
    cadmium::get_messages<model_t::defs::add>(input_bags).assign(std::initializer_list<int>{1, 2, 3, 4});
    cadmium::get_messages<model_t::defs::reset>(input_bags).clear();
    
    //advance simulator
    s.advance_simulation(3.0f, input_bags);
    BOOST_CHECK(s.next() == std::numeric_limits<float>::infinity());

    //external input in reset triggers a reset
    cadmium::get_messages<model_t::defs::add>(input_bags).clear();
    cadmium::get_messages<model_t::defs::reset>(input_bags).emplace_back();
    s.advance_simulation(4.0f, input_bags); //here time is referring to absolute chronology, we are in simulation context.
    BOOST_CHECK(s.next() == 4.0f );

    //out provides the accumulated result
    auto o = s.collect_outputs(4.0f);
    BOOST_REQUIRE(cadmium::get_messages<model_t::defs::sum>(o).size() == 1);
    BOOST_CHECK(cadmium::get_messages<model_t::defs::sum>(o).at(0) == 10);
    s.advance_simulation(4.0f, empty_input);
    BOOST_CHECK(s.next() == std::numeric_limits<float>::infinity());

    //internal transition resets counter
    s.advance_simulation(5.0f, input_bags);
    BOOST_CHECK(s.next() == 5.0f);
    o = s.collect_outputs(5.0f);
    BOOST_REQUIRE(cadmium::get_messages<model_t::defs::sum>(o).size() == 1);
    BOOST_CHECK(cadmium::get_messages<model_t::defs::sum>(o).at(0) == 0);
    s.advance_simulation(5.0f, empty_input);
    BOOST_CHECK(s.next() == std::numeric_limits<float>::infinity());

    //simultaneous external input in both ports increments and schedules reset
    cadmium::get_messages<model_t::defs::add>(input_bags).assign(std::initializer_list<int>{1, 2, 3, 4});
    s.advance_simulation(6.0f, input_bags);
    BOOST_CHECK(s.next() == 6.0f);
    o = s.collect_outputs(6.0f);
    BOOST_REQUIRE(cadmium::get_messages<model_t::defs::sum>(o).size() == 1);
    BOOST_CHECK(cadmium::get_messages<model_t::defs::sum>(o).at(0) == 10);
    s.advance_simulation(6.0f, empty_input);
    BOOST_CHECK(s.next() == std::numeric_limits<float>::infinity());
}

BOOST_AUTO_TEST_CASE( accumulator_simulation_throws_test ){
    //construct a simulator for an accumulator
    using simulator_t= cadmium::engine::simulator<int_accumulator, float>;
    using model_t=simulator_t::model_type;
    simulator_t  s{0.0f};
    BOOST_CHECK(s.next()==std::numeric_limits<float>::infinity());

    //crate the tuple for sending the messages
    typename cadmium::make_message_bags<model_t::input_ports>::type input_bags;
    typename cadmium::make_message_bags<model_t::input_ports>::type empty_input;
    //insert values to add port and reset to empty
    cadmium::get_messages<model_t::defs::add>(input_bags).assign(std::initializer_list<int>{1, 2, 3, 4});
    cadmium::get_messages<model_t::defs::reset>(input_bags).emplace_back();

    //try to obtain output when no internal transition is scheduled
    BOOST_CHECK_THROW(s.collect_outputs(6.0f), std::domain_error);
    //assuming strong exceptions guarantees
    //advance simulator
    s.advance_simulation(3.0f, input_bags);
    BOOST_CHECK(s.next() == 3.0f);

    //try to input in the past of current time
    BOOST_CHECK_THROW(s.advance_simulation(2.0f, input_bags), std::domain_error);
    //try to input later than next scheduled internal event
    BOOST_CHECK_THROW(s.advance_simulation(4.0f, input_bags), std::domain_error);

    //execute expected internal transition
    s.advance_simulation(3.0f, empty_input);
    BOOST_CHECK(s.next()==std::numeric_limits<float>::infinity());
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE_END()
