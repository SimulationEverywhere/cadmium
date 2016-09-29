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
#include<boost/test/unit_test.hpp>

#include<cadmium/modeling/message_bag.hpp>
#include<cadmium/basic_model/accumulator.hpp>
#include<cmath>


BOOST_AUTO_TEST_SUITE( pdevs_basic_models_suite )
    BOOST_AUTO_TEST_SUITE( pdevs_accumulator_suite )

        using floating_accumulator=cadmium::basic_models::accumulator<float, float>;

        BOOST_AUTO_TEST_CASE( it_is_constructable_test )
        {
            BOOST_REQUIRE_NO_THROW( floating_accumulator{} );
        }

        BOOST_AUTO_TEST_CASE( time_advance_is_infinite_after_internal_transition_test )
        {
            auto g = floating_accumulator();
            //setting state
            g.state = std::make_tuple(1.0f, true); //accumulated one and running a reset
            //checking time_advance before and after internal transition
            BOOST_CHECK_EQUAL(0.0f, g.time_advance());
            g.internal_transition();
            BOOST_CHECK(std::isinf(g.time_advance()));
            //validating final state
            BOOST_CHECK_EQUAL(0.0f, std::get<float>(g.state));
            BOOST_CHECK_EQUAL(false, std::get<bool>(g.state));
        }

        BOOST_AUTO_TEST_CASE( it_throws_on_call_to_internal_transition_at_non_reset_state_test )
        {
            auto g = floating_accumulator();
            //setting state
            g.state = std::make_tuple(1.0f, false); //accumulated one and not running a reset
            BOOST_CHECK(std::isinf(g.time_advance()));

            BOOST_CHECK_THROW( g.internal_transition(), std::logic_error);
        }

        BOOST_AUTO_TEST_CASE( it_throws_on_call_to_external_transition_on_reset_state_test )
        {
            auto g = floating_accumulator();
            //setting state
            g.state = std::make_tuple(1.0f, true); //accumulated one and running a reset
            //checking time_advance before and after internal transition
            BOOST_CHECK_EQUAL(0.0f, g.time_advance());

            typename cadmium::make_message_bags<floating_accumulator::input_ports>::type bags;
            cadmium::get_messages<typename floating_accumulator::add>(bags).push_back(5.0f);
            BOOST_CHECK_THROW( g.external_transition(1.0f, bags), std::logic_error);
        }

        BOOST_AUTO_TEST_CASE( output_function_throws_when_not_in_reset_state_test )
        {
            auto g = floating_accumulator();
            //setting state
            g.state = std::make_tuple(1.0f, false); //accumulated one and not running a reset
            BOOST_CHECK_THROW( g.output(), std::logic_error);
        }

        BOOST_AUTO_TEST_CASE( output_function_returns_accumulated_value_test )
        {
            auto g = floating_accumulator();
            //setting state
            g.state = std::make_tuple(10.0f, false); //accumulated one and not running a reset
            //introducing 3 new values, and setting
            typename cadmium::make_message_bags<floating_accumulator::input_ports>::type bags_one;
            cadmium::get_messages<typename floating_accumulator::add>(bags_one).push_back(5.0f);
            g.external_transition(10.0f, bags_one);
            //validate state
            BOOST_CHECK_EQUAL(15.0f, std::get<float>(g.state));
            BOOST_CHECK_EQUAL(false, std::get<bool>(g.state));

            typename cadmium::make_message_bags<floating_accumulator::input_ports>::type bags_two;
            cadmium::get_messages<typename floating_accumulator::add>(bags_two).push_back(3.0f);
            cadmium::get_messages<typename floating_accumulator::add>(bags_two).push_back(7.0f);
            g.external_transition(9.0f, bags_two);
            //validate state
            BOOST_CHECK_EQUAL(25.0f, std::get<float>(g.state));
            BOOST_CHECK_EQUAL(false, std::get<bool>(g.state));

            typename cadmium::make_message_bags<floating_accumulator::input_ports>::type bags_three;
            cadmium::get_messages<typename floating_accumulator::add>(bags_three).push_back(3.0f);
            cadmium::get_messages<typename floating_accumulator::reset>(bags_three).emplace_back();
            g.external_transition(2.0f, bags_three);
            BOOST_CHECK_EQUAL(28.0f, std::get<float>(g.state));
            BOOST_CHECK_EQUAL(true, std::get<bool>(g.state));

            //running confluence, because waiting for an internal here
            typename cadmium::make_message_bags<floating_accumulator::input_ports>::type bags_four;
            cadmium::get_messages<typename floating_accumulator::add>(bags_four).push_back(2.0f);
            g.confluence_transition(0.0f, bags_four);
            BOOST_CHECK_EQUAL(2.0f, std::get<float>(g.state));
            BOOST_CHECK_EQUAL(false, std::get<bool>(g.state));
        }

    BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
