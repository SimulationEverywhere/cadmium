/**
 * Copyright (c) 2013-2016, 
 * Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis
 * Cristina Ruiz Martin
 * Carleton University, Universidad de Valladolid
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
#include<cadmium/basic_model/partial_accumulator.hpp>
#include<cmath>


BOOST_AUTO_TEST_SUITE( pdevs_basic_models_suite )
    BOOST_AUTO_TEST_SUITE( pdevs_Partialaccumulator_suite )
        struct set{            
        };
        using floating_partial_accumulator=cadmium::basic_models::partial_accumulator<float, set, float>;
        using floating_partial_accumulator_defs=cadmium::basic_models::partial_accumulator_defs<float, set>;

        BOOST_AUTO_TEST_CASE( it_is_constructable_test )
        {
            BOOST_REQUIRE_NO_THROW( floating_partial_accumulator{} );
        }

        BOOST_AUTO_TEST_CASE( time_advance_is_infinite_after_internal_transition_test )
        {
            auto g = floating_partial_accumulator();
            //setting state (accum, reset, partial)
            g.state.set_state(1.0f, true, false); //accumulated one and running a reset
            //checking time_advance before and after internal transition
            BOOST_CHECK_EQUAL(0.0f, g.time_advance());
            g.internal_transition();
            BOOST_CHECK(std::isinf(g.time_advance()));
            //validating final state
            BOOST_CHECK_EQUAL(0.0f, g.state.accumulated);
            BOOST_CHECK_EQUAL(false, g.state.on_reset);
            BOOST_CHECK_EQUAL(false, g.state.on_partial);
        }

        BOOST_AUTO_TEST_CASE( it_throws_on_call_to_internal_transition_at_non_reset_state_test_and_non_partial_state )
        {
            auto g = floating_partial_accumulator();
            //setting state
            g.state.set_state(1.0f, false, false); //accumulated one and not running a reset and not sending accumulated value
            //checking time_advence of the state
            BOOST_CHECK(std::isinf(g.time_advance()));
            //checking invalid state for internal
            BOOST_CHECK_THROW( g.internal_transition(), std::logic_error);
        }

        BOOST_AUTO_TEST_CASE( it_throws_on_call_to_external_transition_on_reset_state_test)
        {
            auto g = floating_partial_accumulator();
            //setting state
            g.state.set_state(1.0f, true, false); //accumulated one and running a reset
            //checking time_advence of the state
            BOOST_CHECK_EQUAL(0.0f, g.time_advance());
            //checking invalid state for the external
            typename cadmium::make_message_bags<floating_partial_accumulator::input_ports>::type bags;
            cadmium::get_messages<typename floating_partial_accumulator_defs::add>(bags).push_back(5.0f);
            BOOST_CHECK_THROW( g.external_transition(1.0f, bags), std::logic_error);
        }

        BOOST_AUTO_TEST_CASE( it_throws_on_call_to_external_transition_on_partial_state_test)
        {
            auto g = floating_partial_accumulator();
            //setting state
            g.state.set_state(1.0f, false, true); //accumulated one and running a reset
            //checking time_advence of the state
            BOOST_CHECK_EQUAL(0.0f, g.time_advance());
            //checking invalid state for the external
            typename cadmium::make_message_bags<floating_partial_accumulator::input_ports>::type bags;
            cadmium::get_messages<typename floating_partial_accumulator_defs::add>(bags).push_back(5.0f);
            BOOST_CHECK_THROW( g.external_transition(1.0f, bags), std::logic_error);
        }


        BOOST_AUTO_TEST_CASE( output_function_returns_accumulated_value_test_and_external_combinations )
        {
            auto g = floating_partial_accumulator();
            //setting state
            g.state.set_state(10.0f, false, false); //accumulated ten and not running a reset and not sending output
            
            //introducing new value
            typename cadmium::make_message_bags<floating_partial_accumulator::input_ports>::type bags_one;
            cadmium::get_messages<typename floating_partial_accumulator_defs::add>(bags_one).push_back(5.0f);
            g.external_transition(10.0f, bags_one);
            //validate state
            BOOST_CHECK_EQUAL(15.0f, g.state.accumulated);
            BOOST_CHECK_EQUAL(false, g.state.on_reset);
            BOOST_CHECK_EQUAL(false, g.state.on_partial);
            //checking time_advance after the external transition
            BOOST_CHECK(std::isinf(g.time_advance()));

            //introducing new value
            typename cadmium::make_message_bags<floating_partial_accumulator::input_ports>::type bags_two;
            cadmium::get_messages<typename floating_partial_accumulator_defs::add>(bags_two).push_back(3.0f);
            cadmium::get_messages<typename floating_partial_accumulator_defs::add>(bags_two).push_back(7.0f);
            g.external_transition(9.0f, bags_two);
            //validate state
            BOOST_CHECK_EQUAL(25.0f, g.state.accumulated);
            BOOST_CHECK_EQUAL(false, g.state.on_reset);
            BOOST_CHECK_EQUAL(false, g.state.on_partial);
            //checking time_advance after the external transition
            BOOST_CHECK(std::isinf(g.time_advance()));
            
            //introducing new value and seting partial
            typename cadmium::make_message_bags<floating_partial_accumulator::input_ports>::type bags_three;
            cadmium::get_messages<typename floating_partial_accumulator_defs::add>(bags_three).push_back(3.0f);
            cadmium::get_messages<typename floating_partial_accumulator_defs::partial>(bags_three).emplace_back();
            g.external_transition(2.0f, bags_three);
            BOOST_CHECK_EQUAL(28.0f, g.state.accumulated);
            BOOST_CHECK_EQUAL(false, g.state.on_reset);
            BOOST_CHECK_EQUAL(true, g.state.on_partial);
            //checking time_advance after the external transition
            BOOST_CHECK_EQUAL(0.0f, g.time_advance());
            //validate output
            typename cadmium::make_message_bags<floating_partial_accumulator::output_ports>::type outmb1;
            outmb1 = g.output();
            typename cadmium::make_message_bags<floating_partial_accumulator::output_ports>::type outmb_expected1;
            cadmium::get_messages<typename floating_partial_accumulator_defs::sum>(outmb_expected1).push_back(28.0f);
            BOOST_CHECK_EQUAL(1, cadmium::get_messages<typename floating_partial_accumulator_defs::sum>(outmb1).size());
            BOOST_CHECK_EQUAL(1, cadmium::get_messages<typename floating_partial_accumulator_defs::sum>(outmb_expected1).size());
            BOOST_CHECK_EQUAL(cadmium::get_messages<typename floating_partial_accumulator_defs::sum>(outmb_expected1)[0], cadmium::get_messages<typename floating_partial_accumulator_defs::sum>(outmb1)[0]);

            //running confluence, because waiting for an internal here external is setting reset
            typename cadmium::make_message_bags<floating_partial_accumulator::input_ports>::type bags_four;
            cadmium::get_messages<typename floating_partial_accumulator_defs::reset>(bags_four).emplace_back();
            g.confluence_transition(0.0f, bags_four);
            BOOST_CHECK_EQUAL(28.0f, g.state.accumulated);
            BOOST_CHECK_EQUAL(true, g.state.on_reset);
            BOOST_CHECK_EQUAL(false, g.state.on_partial);
            //checking time_advance after the confluence transition
            BOOST_CHECK_EQUAL(0.0f, g.time_advance());            
            //validate output
            typename cadmium::make_message_bags<floating_partial_accumulator::output_ports>::type outmb2;
            outmb2 = g.output();
            BOOST_CHECK_EQUAL(true, cadmium::get_messages<typename floating_partial_accumulator_defs::sum>(outmb2).empty());



            //introducing new value. Running confluence as time_advance is cero
            typename cadmium::make_message_bags<floating_partial_accumulator::input_ports>::type bags_five;
            cadmium::get_messages<typename floating_partial_accumulator_defs::add>(bags_five).push_back(5.0f);
            g.confluence_transition(10.0f, bags_five);
            //validate state
            BOOST_CHECK_EQUAL(5.0f, g.state.accumulated);
            BOOST_CHECK_EQUAL(false, g.state.on_reset);
            BOOST_CHECK_EQUAL(false, g.state.on_partial);
            //checking time_advance after the external transition
            BOOST_CHECK(std::isinf(g.time_advance()));


            //introducing new value and seting reset
            typename cadmium::make_message_bags<floating_partial_accumulator::input_ports>::type bags_six;
            cadmium::get_messages<typename floating_partial_accumulator_defs::add>(bags_six).push_back(3.0f);
            cadmium::get_messages<typename floating_partial_accumulator_defs::reset>(bags_six).emplace_back();
            g.external_transition(2.0f, bags_six);
            //validate state
            BOOST_CHECK_EQUAL(8.0f, g.state.accumulated);
            BOOST_CHECK_EQUAL(true, g.state.on_reset);
            BOOST_CHECK_EQUAL(false, g.state.on_partial);
            //checking time_advance after the external transition
            BOOST_CHECK_EQUAL(0.0f, g.time_advance());
            //validate output
            typename cadmium::make_message_bags<floating_partial_accumulator::output_ports>::type outmb3;
            outmb3 = g.output();
            BOOST_CHECK_EQUAL(true, cadmium::get_messages<typename floating_partial_accumulator_defs::sum>(outmb3).empty());
            //runing internal as time_advance is cero
            g.internal_transition();
            //validate state
            BOOST_CHECK_EQUAL(0.0f, g.state.accumulated);
            BOOST_CHECK_EQUAL(false, g.state.on_reset);
            BOOST_CHECK_EQUAL(false, g.state.on_partial);
            //checking time_advance after the internal transition
            BOOST_CHECK(std::isinf(g.time_advance()));

            //introducing new value and seting reset and seting partial
            typename cadmium::make_message_bags<floating_partial_accumulator::input_ports>::type bags_seven;
            cadmium::get_messages<typename floating_partial_accumulator_defs::add>(bags_seven).push_back(13.0f);
            cadmium::get_messages<typename floating_partial_accumulator_defs::partial>(bags_seven).emplace_back();
            cadmium::get_messages<typename floating_partial_accumulator_defs::reset>(bags_seven).emplace_back();
            g.external_transition(2.0f, bags_seven);
            //validate state
            BOOST_CHECK_EQUAL(13.0f, g.state.accumulated);
            BOOST_CHECK_EQUAL(true, g.state.on_reset);
            BOOST_CHECK_EQUAL(true, g.state.on_partial);
            //checking time_advance after the external transition
            BOOST_CHECK_EQUAL(0.0f, g.time_advance());
            //validate output
            typename cadmium::make_message_bags<floating_partial_accumulator::output_ports>::type outmb4;
            outmb4 = g.output();
            typename cadmium::make_message_bags<floating_partial_accumulator::output_ports>::type outmb_expected4;
            cadmium::get_messages<typename floating_partial_accumulator_defs::sum>(outmb_expected4).push_back(13.0f);
            BOOST_CHECK_EQUAL(1, cadmium::get_messages<typename floating_partial_accumulator_defs::sum>(outmb4).size());
            BOOST_CHECK_EQUAL(1, cadmium::get_messages<typename floating_partial_accumulator_defs::sum>(outmb_expected4).size());
            BOOST_CHECK_EQUAL(cadmium::get_messages<typename floating_partial_accumulator_defs::sum>(outmb_expected4)[0], cadmium::get_messages<typename floating_partial_accumulator_defs::sum>(outmb4)[0]);
            //runing internal as time_advance is cero
            g.internal_transition();
            //validate state
            BOOST_CHECK_EQUAL(0.0f, g.state.accumulated);
            BOOST_CHECK_EQUAL(false, g.state.on_reset);
            BOOST_CHECK_EQUAL(false, g.state.on_partial);
            //checking time_advance after the internal transition
            BOOST_CHECK(std::isinf(g.time_advance()));

        }

    BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()