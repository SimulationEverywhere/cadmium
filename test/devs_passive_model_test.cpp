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


#define BOOST_TEST_DYN_LINK

#include<boost/test/unit_test.hpp>

#include<cadmium/basic_model/devs/passive.hpp>
#include<cadmium/concept/concept_helpers.hpp>

#include<cmath>
#include<stdexcept>


BOOST_AUTO_TEST_SUITE(devs_basic_models_suite)
    BOOST_AUTO_TEST_SUITE(devs_passive_suite)

        template<typename TIME>
        using floating_passive=cadmium::basic_models::devs::passive<float, TIME>;
        using floating_passive_defs=cadmium::basic_models::devs::passive_defs<float>;

        BOOST_AUTO_TEST_CASE(it_is_atomic_test) {
            BOOST_CHECK(cadmium::concept::is_atomic<floating_passive>::value());
        }

        BOOST_AUTO_TEST_CASE(it_is_constructable_test) {
            BOOST_REQUIRE_NO_THROW(floating_passive<float>{});
        }

        BOOST_AUTO_TEST_CASE(it_throws_on_call_to_internal_transition_test) {
            auto p = floating_passive<float>();
            BOOST_CHECK_THROW(p.internal_transition(), std::logic_error);
        }

        BOOST_AUTO_TEST_CASE(it_throws_on_call_to_output_function_test) {
            auto p = floating_passive<float>();
            BOOST_CHECK_THROW(p.output(), std::logic_error);
        }

        BOOST_AUTO_TEST_CASE(call_to_external_transition_keeps_infinite_time_advance_test) {
            auto p = floating_passive<float>();
            bool is_inf_ta = std::isinf(p.time_advance());
            BOOST_CHECK_MESSAGE(is_inf_ta, "Passive model is not in passive state");
            typename cadmium::make_message_box<floating_passive<float>::input_ports>::type input;
            cadmium::get_message<typename floating_passive_defs::in>(input).emplace(1);
            BOOST_CHECK_NO_THROW(p.external_transition(5.0, input));
            is_inf_ta = std::isinf(p.time_advance());
            BOOST_CHECK_MESSAGE(is_inf_ta, "Passive model is not in passive state");
        }

    BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
