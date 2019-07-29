/**
 * Copyright (c) 2017, Damian Vicino, Laouen M. L. Belloli
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
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/modeling/dynamic_atomic.hpp>
#include <cadmium/engine/pdevs_dynamic_simulator.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>


template<typename TIME>
using int_accumulator=cadmium::basic_models::pdevs::accumulator<int, TIME>;
using int_accumulator_defs=cadmium::basic_models::pdevs::accumulator_defs<int>;

BOOST_AUTO_TEST_SUITE( pdevs_dynamic_simulator_suite )

BOOST_AUTO_TEST_SUITE( pdevs_accumulator_suite )
BOOST_AUTO_TEST_CASE( accumulator_model_dynamic_simulation_test )
{
//This test is suppose to pass only in CPP17 compilers, skipping in older compilers
#if __cplusplus > 201702 && defined (DYNAMIC_ENGINE)
    //construct a simulator for an accumulator
    std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<float>> upModel = cadmium::dynamic::translate::make_dynamic_atomic_model<int_accumulator, float>();
    cadmium::dynamic::engine::simulator<float, cadmium::logger::not_logger> s(upModel);

    s.init(0.0f);

    BOOST_CHECK(s.next()==std::numeric_limits<float>::infinity());

    using input_ports=int_accumulator<float>::input_ports;
    using in_bags_type=typename cadmium::make_message_bags<input_ports>::type;

    //crate the map for sending/receiving the messages
    cadmium::message_bag<int_accumulator_defs::add> bag_0;
    cadmium::message_bag<int_accumulator_defs::reset> bag_1;
    cadmium::message_bag<int_accumulator_defs::sum> output;

    bag_0.messages.assign(std::initializer_list<int>{1, 2, 3, 4});
    bag_1.messages.clear();

    cadmium::dynamic::message_bags input_bags;
    cadmium::dynamic::message_bags empty_input = cadmium::dynamic::modeling::create_empty_message_bags<in_bags_type>();

    input_bags[typeid(int_accumulator_defs::add)] = bag_0;
    input_bags[typeid(int_accumulator_defs::reset)] = bag_1;

    //advance simulator
    s._inbox = input_bags;
    s.advance_simulation(3.0f);
    BOOST_CHECK(s.next() == std::numeric_limits<float>::infinity());

    //external input in reset triggers a reset
    bag_0.messages.clear();
    bag_1.messages.emplace_back();

    input_bags[typeid(int_accumulator_defs::add)] = bag_0;
    input_bags[typeid(int_accumulator_defs::reset)] = bag_1;

    s._inbox = input_bags;
    s.advance_simulation(4.0f); //here time is referring to absolute chronology, we are in simulation context.
    BOOST_CHECK(s.next() == 4.0f );

    //out provides the accumulated result
    s.collect_outputs(4.0f);
    auto o = s.outbox();
    output = boost::any_cast<cadmium::message_bag<int_accumulator_defs::sum>>(o.at(typeid(int_accumulator_defs::sum)));
    BOOST_CHECK(output.messages.size() == 1);
    BOOST_CHECK(output.messages.at(0) == 10);

    s._inbox = empty_input;
    s.advance_simulation(4.0f);
    BOOST_CHECK(s.next() == std::numeric_limits<float>::infinity());

    //internal transition resets counter
    s._inbox = input_bags;
    s.advance_simulation(5.0f);
    BOOST_CHECK(s.next() == 5.0f);
    s.collect_outputs(5.0f);
    o = s.outbox();
    output = boost::any_cast<cadmium::message_bag<int_accumulator_defs::sum>>(o.at(typeid(int_accumulator_defs::sum)));
    BOOST_CHECK(output.messages.size() == 1);
    BOOST_CHECK(output.messages.at(0) == 0);
    s._inbox = empty_input;
    s.advance_simulation(5.0f);
    BOOST_CHECK(s.next() == std::numeric_limits<float>::infinity());

    //simultaneous external input in both ports increments and schedules reset
    bag_0.messages.assign(std::initializer_list<int>{1, 2, 3, 4});
    input_bags[typeid(int_accumulator_defs::add)] = bag_0;

    s._inbox = input_bags;
    s.advance_simulation(6.0f);
    BOOST_CHECK(s.next() == 6.0f);
    s.collect_outputs(6.0f);
    o = s.outbox();
    output = boost::any_cast<cadmium::message_bag<int_accumulator_defs::sum>>(o.at(typeid(int_accumulator_defs::sum)));
    BOOST_CHECK(output.messages.size() == 1);
    BOOST_CHECK(output.messages.at(0) == 10);

    s._inbox = empty_input;
    s.advance_simulation(6.0f);
    BOOST_CHECK(s.next() == std::numeric_limits<float>::infinity());
#else
    BOOST_WARN_MESSAGE(false, "Skippping accumulator_model_dynamic_simulation_test test because compiler is not C++17 compliant");
#endif
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE_END()

