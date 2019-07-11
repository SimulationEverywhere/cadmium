/**
 * Copyright (c) 2013-2019, Damian Vicino
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

#include<cadmium/modeling/message_box.hpp>
#include<cadmium/basic_model/devs/accumulator.hpp>
#include<cadmium/concept/concept_helpers.hpp>
#include<cmath>


BOOST_AUTO_TEST_SUITE(devs_basic_models_suite)
    BOOST_AUTO_TEST_SUITE(devs_accumulator_suite)

        template<typename TIME>
        using floating_accumulator=cadmium::basic_models::devs::accumulator<float, TIME>;
        using floating_accumulator_defs=cadmium::basic_models::devs::accumulator_defs<float>;

        BOOST_AUTO_TEST_CASE(is_it_atomic_test) {
            BOOST_CHECK(cadmium::concept::is_atomic<floating_accumulator>::value());
        }

        BOOST_AUTO_TEST_CASE(it_is_constructable_test) {
            BOOST_REQUIRE_NO_THROW(floating_accumulator<float>{});
        }


        BOOST_AUTO_TEST_CASE(time_advance_is_infinite_after_internal_transition_test) {
            auto a = floating_accumulator<float>();
            //setting state as accumulate one and run a reset
            a.state = std::make_tuple(1.0f, true);
            //checking time_advance before and after internal transition
            BOOST_CHECK_EQUAL(0.0f, a.time_advance());
            a.internal_transition();
            BOOST_CHECK(std::isinf(a.time_advance()));
            //validating final state
            BOOST_CHECK_EQUAL(0.0f, std::get<float>(a.state));
            BOOST_CHECK_EQUAL(false, std::get<bool>(a.state));
        }

        BOOST_AUTO_TEST_CASE(it_throws_on_call_to_internal_transition_at_non_reset_state_test) {
            auto a = floating_accumulator<float>();
            //setting state to accumulate one and not on reset
            a.state = std::make_tuple(1.0f, false);
            BOOST_CHECK(std::isinf(a.time_advance()));

            BOOST_CHECK_THROW(a.internal_transition(), std::logic_error);
        }

        BOOST_AUTO_TEST_CASE(it_throws_on_call_to_external_transition_on_reset_state_test) {
            auto a = floating_accumulator<float>();
            //setting state to accumulate one and on reset
            a.state = std::make_tuple(1.0f, true);
            //checking time_advance before and after internal transition
            BOOST_CHECK_EQUAL(0.0f, a.time_advance());

            typename cadmium::make_message_box<floating_accumulator<float>::input_ports>::type box;
            cadmium::get_message<floating_accumulator_defs::add>(box).emplace(5.0f);
            BOOST_CHECK_THROW(a.external_transition(1.0f, box), std::logic_error);
        }

        BOOST_AUTO_TEST_CASE(output_function_throws_when_not_in_reset_state_test) {
            auto a = floating_accumulator<float>();
            //setting state
            a.state = std::make_tuple(1.0f, false); //accumulated one and not running a reset
            BOOST_CHECK_THROW(a.output(), std::logic_error);
        }

        BOOST_AUTO_TEST_CASE(output_function_returns_accumulated_value_test) {
            auto a = floating_accumulator<float>();
            //setting state
            a.state = std::make_tuple(10.0f, false); //accumulated one and not running a reset
            //introducing 3 new values, and setting
            typename cadmium::make_message_box<floating_accumulator<float>::input_ports>::type input;
            cadmium::get_message<typename floating_accumulator_defs::add>(input).emplace(5.0f);
            a.external_transition(10.0f, input);
            //validate state
            BOOST_CHECK_EQUAL(15.0f, std::get<float>(a.state));
            BOOST_CHECK_EQUAL(false, std::get<bool>(a.state));

            cadmium::get_message<typename floating_accumulator_defs::add>(input).emplace(3.0f);
            a.external_transition(9.0f, input);
            //validate state
            BOOST_CHECK_EQUAL(18.0f, std::get<float>(a.state));
            BOOST_CHECK_EQUAL(false, std::get<bool>(a.state));

            cadmium::get_message<typename floating_accumulator_defs::add>(input).emplace(7.0f);
            a.external_transition(9.0f, input);
            //validate state
            BOOST_CHECK_EQUAL(25.0f, std::get<float>(a.state));
            BOOST_CHECK_EQUAL(false, std::get<bool>(a.state));

            //testing a reset to make internal transition to execute next
            cadmium::get_message<typename floating_accumulator_defs::add>(input).emplace(3.0f);
            cadmium::get_message<typename floating_accumulator_defs::reset>(input).emplace();
            a.external_transition(2.0f, input);
            BOOST_CHECK_EQUAL(28.0f, std::get<float>(a.state));
            BOOST_CHECK_EQUAL(true, std::get<bool>(a.state));

            //validate output
            auto outmb1 = a.output();
            BOOST_CHECK(cadmium::get_message<typename floating_accumulator_defs::sum>(outmb1).has_value());
            BOOST_CHECK_EQUAL(28.0f, cadmium::get_message<typename floating_accumulator_defs::sum>(outmb1).value());
        }

    BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
