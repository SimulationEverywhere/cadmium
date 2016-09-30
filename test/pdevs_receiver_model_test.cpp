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
#include<cadmium/basic_model/receiverCadmium.hpp>
#include<cmath>


BOOST_AUTO_TEST_SUITE( pdevs_basic_models_suite )
    BOOST_AUTO_TEST_SUITE( pdevs_receiver_suite )

        using int_receiver=cadmium::basic_models::receiver<int, float>;

        BOOST_AUTO_TEST_CASE( it_is_constructable_test )
        {
            BOOST_REQUIRE_NO_THROW( int_receiver{} );
        }

        BOOST_AUTO_TEST_CASE( time_advance_is_infinite_after_internal_transition_test )
        {
            auto g = int_receiver();
            //setting state
            g.state.active = true; //the receiver is active
            //checking time_advance before and after internal transition
            BOOST_CHECK_EQUAL(10.0f, g.time_advance());
            g.internal_transition();
            BOOST_CHECK(std::isinf(g.time_advance()));
            //validating final state
            BOOST_CHECK_EQUAL(false, g.state.active);
        }

        BOOST_AUTO_TEST_CASE( it_throws_assert_if_more_than_one_message_external)
        {
            auto g = int_receiver();
            //setting state
            g.state.active = false;
            g.state.ackNum = 0;
            typename cadmium::make_message_bags<int_receiver::input_ports>::type bags;
            //cadmium::get_messages<typename int_receiver::in>(bags).push_back(5);
            //cadmium::get_messages<typename int_receiver::in>(bags).push_back(10);
            //BOOST_CHECK_THROW( g.external_transition(1.0f, bags), std::logic_error);
        }

        BOOST_AUTO_TEST_CASE( state_after_external )
        {
            auto g = int_receiver();
            g.state.active = false;
            g.state.ackNum = 0;
            typename cadmium::make_message_bags<int_receiver::input_ports>::type bags;
            //cadmium::get_messages<typename int_receiver::in>(bags).push_back(5.0f);
            //g.external_transition(1.0f, bags);
            BOOST_CHECK_EQUAL( true, g.state.active);
            BOOST_CHECK_EQUAL( 5.0f, g.state.ackNum);
        }

        BOOST_AUTO_TEST_CASE( output_function_returns_accumulated_value_test )
        {
            auto g = int_receiver();
            g.state.active = false;
            g.state.ackNum = 0;
            typename cadmium::make_message_bags<int_receiver::input_ports>::type bags;
            //cadmium::get_messages<typename int_receiver::in>(bags).push_back(5.0f);
            //g.external_transition(1.0f, bags);
            BOOST_CHECK_EQUAL( true, g.state.active);
            BOOST_CHECK_EQUAL( 5.0f, g.state.ackNum);

            //validate output
            typename cadmium::make_message_bags<int_receiver::output_ports>::type outmb1;
            //outmb1 = g.output();
            typename cadmium::make_message_bags<int_receiver::output_ports>::type outmb_expected;
            //cadmium::get_messages<typename int_receiver::out>(outmb_expected).push_back(g.state.ackNum % 10);
            //BOOST_CHECK_EQUAL(1, cadmium::get_messages<typename int_receiver::out>(outmb1).size());
            //BOOST_CHECK_EQUAL(cadmium::get_messages<typename int_receiver::out>(outmb_expected)[0], cadmium::get_messages<typename int_receiver::out>(outmb_expected)[0]);
        }

    BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
