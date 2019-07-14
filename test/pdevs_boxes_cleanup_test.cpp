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

#include <boost/test/unit_test.hpp>
#include <cadmium/logger/tuple_to_ostream.hpp>
#include <cadmium/basic_model/pdevs/int_generator_one_sec.hpp>
#include <cadmium/basic_model/pdevs/filter_first_output.hpp>
#include <cadmium/basic_model/pdevs/accumulator.hpp>
#include <cadmium/modeling/coupling.hpp>
#include <cadmium/engine/pdevs_runner.hpp>


/**
  * This test suite is reproducing a bug that was reported in 2017 that under certain circumstances
  * events were received multiple times in the inbox of stale models.
  * Messages were stale and read multiple times from inbox in some cases (simple_inbox_cleanup_bug_test)
  * and messages were stale and routed multiples times from outboxes in other cases (simple_outbox_cleanup_bug_test)
  */
BOOST_AUTO_TEST_SUITE(inbox_cleanup_test_suite)
    namespace BM=cadmium::basic_models::pdevs;

//Defining a stream to capture logs for validation
    namespace {
        std::ostringstream oss;

        struct oss_test_sink_provider {
            static std::ostream &sink() {
                return oss;
            }
        };
    }

//loggers in output to oss for test
    using global_time=cadmium::logger::logger<cadmium::logger::logger_global_time, cadmium::logger::formatter<float>, oss_test_sink_provider>;
    using state=cadmium::logger::logger<cadmium::logger::logger_state, cadmium::logger::formatter<float>, oss_test_sink_provider>;
    using routing=cadmium::logger::logger<cadmium::logger::logger_message_routing, cadmium::logger::formatter<float>, oss_test_sink_provider>;
    using log_time_and_state=cadmium::logger::multilogger<state, global_time>;

//helper function for checking matches in log
    int count_matches(std::string nail, std::string haystack) {
        int count = 0;
        size_t nPos = haystack.find(nail, 0); // fist occurrence
        while (nPos != std::string::npos) {
            count++;
            nPos = haystack.find(nail, nPos + 1);
        }
        return count;
    }


//Atomic models used by the tests
    template<typename TIME>
    struct first_receiver : public BM::filter_first_output<TIME> {
    };
    template<typename TIME>
    struct filter_one : public BM::filter_first_output<TIME> {
    };
    template<typename TIME>
    using gen=BM::int_generator_one_sec<TIME>;
    template<typename TIME>
    using acc=BM::accumulator<int, TIME>;

//coupled models for simple_inbox_cleanup_bug_test
//C1 (Gen->Filter->)
    struct C1_out : public cadmium::out_port<int> {
    };
    using ips_C1 = std::tuple<>;
    using ops_C1 = std::tuple<C1_out>;
    using submodels_C1 =cadmium::modeling::models_tuple<gen, filter_one>;
    using eics_C1 = std::tuple<>;
    using eocs_C1 = std::tuple<cadmium::modeling::EOC<filter_one, BM::filter_first_output_defs::out, C1_out>>;
    using ics_C1 = std::tuple<
            cadmium::modeling::IC<gen, BM::int_generator_one_sec_defs::out, filter_one, BM::filter_first_output_defs::in>
    >;
    template<typename TIME>
    struct C1
            : public cadmium::modeling::pdevs::coupled_model<TIME, ips_C1, ops_C1, submodels_C1, eics_C1, eocs_C1, ics_C1> {
    };
//C2 (->Accum->)
    struct C2_in : public cadmium::in_port<int> {
    };
    struct C2_out : public cadmium::out_port<int> {
    };
    using ips_C2 = std::tuple<C2_in>;
    using ops_C2 = std::tuple<C2_out>;
    using submodels_C2 =cadmium::modeling::models_tuple<acc>;
    using eics_C2 = std::tuple<cadmium::modeling::EIC<C2_in, acc, BM::accumulator_defs<int>::add>>;
    using eocs_C2 = std::tuple<cadmium::modeling::EOC<acc, BM::accumulator_defs<int>::sum, C2_out>>;
    using ics_C2 = std::tuple<>;
    template<typename TIME>
    struct C2
            : public cadmium::modeling::pdevs::coupled_model<TIME, ips_C2, ops_C2, submodels_C2, eics_C2, eocs_C2, ics_C2> {
    };

//C3 (->C2->)
    struct C3_in : public cadmium::in_port<int> {
    };
    struct C3_out : public cadmium::out_port<int> {
    };
    using ips_C3 = std::tuple<C3_in>;
    using ops_C3 = std::tuple<C3_out>;
    using submodels_C3 =cadmium::modeling::models_tuple<C2>;
    using eics_C3 = std::tuple<cadmium::modeling::EIC<C3_in, C2, C2_in>>;
    using eocs_C3 = std::tuple<cadmium::modeling::EOC<C2, C2_out, C3_out>>;
    using ics_C3 = std::tuple<>;
    template<typename TIME>
    struct C3
            : public cadmium::modeling::pdevs::coupled_model<TIME, ips_C3, ops_C3, submodels_C3, eics_C3, eocs_C3, ics_C3> {
    };

//TOP (C1->C3->)
    struct top_out : public cadmium::out_port<int> {
    };
    using ips_TOP = std::tuple<>;
    using ops_TOP = std::tuple<top_out>;
    using submodels_TOP =cadmium::modeling::models_tuple<C1, C3>;
    using eics_TOP = std::tuple<>;
    using eocs_TOP = std::tuple<cadmium::modeling::EOC<C3, C3_out, top_out>>;
    using ics_TOP = std::tuple<cadmium::modeling::IC<C1, C1_out, C3, C3_in>>;
    template<typename TIME>
    struct CTOP
            : public cadmium::modeling::pdevs::coupled_model<TIME, ips_TOP, ops_TOP, submodels_TOP, eics_TOP, eocs_TOP, ics_TOP> {
    };

    BOOST_AUTO_TEST_CASE(simple_inbox_cleanup_bug_test) {
        std::string expected_initial_state = "State for model cadmium::basic_models::pdevs::accumulator<int, float> is [0, 0]";
        std::string expected_accumulation_of_one = "State for model cadmium::basic_models::pdevs::accumulator<int, float> is [1, 0]";
        std::string accum_states = "State for model cadmium::basic_models::pdevs::accumulator";
        cadmium::engine::runner<float, CTOP, log_time_and_state> r{0.0};
        r.run_until(5.0);

        int count_initial_states = count_matches(expected_initial_state, oss.str());
        int count_expected_accumulation = count_matches(expected_accumulation_of_one, oss.str());
        int count_accum_states = count_matches(accum_states, oss.str());
        // The accumulator has to increment the count once only
        BOOST_CHECK_EQUAL(count_initial_states + count_expected_accumulation, count_accum_states);
    }

//Extra filter model for simple_outbox_cleanup_bug_test
    template<typename TIME>
    struct second_filter_one : public BM::filter_first_output<TIME> {
    };

//coupled models for simple_outbox_cleanup_bug_test
//D1 (Gen->Filter_one->)
    struct D1_out : public cadmium::out_port<int> {
    };
    using ips_D1 = std::tuple<>;
    using ops_D1 = std::tuple<D1_out>;
    using submodels_D1 =cadmium::modeling::models_tuple<gen, filter_one>;
    using eics_D1 = std::tuple<>;
    using eocs_D1 = std::tuple<cadmium::modeling::EOC<filter_one, BM::filter_first_output_defs::out, D1_out>>;
    using ics_D1 = std::tuple<
            cadmium::modeling::IC<gen, BM::int_generator_one_sec_defs::out, filter_one, BM::filter_first_output_defs::in>
    >;
    template<typename TIME>
    struct D1
            : public cadmium::modeling::pdevs::coupled_model<TIME, ips_D1, ops_D1, submodels_D1, eics_D1, eocs_D1, ics_D1> {
    };

//D2 (->second_Filter_one->Accum->)
    struct D2_in : public cadmium::in_port<int> {
    };
    struct D2_out : public cadmium::out_port<int> {
    };
    using ips_D2 = std::tuple<D2_in>;
    using ops_D2 = std::tuple<D2_out>;
    using submodels_D2 =cadmium::modeling::models_tuple<second_filter_one, acc>;
    using eics_D2 = std::tuple<cadmium::modeling::EIC<D2_in, second_filter_one, BM::filter_first_output_defs::in>>;
    using eocs_D2 = std::tuple<cadmium::modeling::EOC<acc, BM::accumulator_defs<int>::sum, D2_out>>;
    using ics_D2 = std::tuple<cadmium::modeling::IC<second_filter_one, BM::filter_first_output_defs::out, acc, BM::accumulator_defs<int>::add>>;
    template<typename TIME>
    struct D2
            : public cadmium::modeling::pdevs::coupled_model<TIME, ips_D2, ops_D2, submodels_D2, eics_D2, eocs_D2, ics_D2> {
    };

//D3 (->D2->)
    struct D3_in : public cadmium::in_port<int> {
    };
    struct D3_out : public cadmium::out_port<int> {
    };
    using ips_D3 = std::tuple<D3_in>;
    using ops_D3 = std::tuple<D3_out>;
    using submodels_D3 =cadmium::modeling::models_tuple<D2>;
    using eics_D3 = std::tuple<cadmium::modeling::EIC<D3_in, D2, D2_in>>;
    using eocs_D3 = std::tuple<cadmium::modeling::EOC<D2, D2_out, D3_out>>;
    using ics_D3 = std::tuple<>;
    template<typename TIME>
    struct D3
            : public cadmium::modeling::pdevs::coupled_model<TIME, ips_D3, ops_D3, submodels_D3, eics_D3, eocs_D3, ics_D3> {
    };

//DTOP (D1->D3->)
    struct DTOP_out : public cadmium::out_port<int> {
    };
    using ips_DTOP = std::tuple<>;
    using ops_DTOP = std::tuple<DTOP_out>;
    using submodels_DTOP =cadmium::modeling::models_tuple<D1, D3>;
    using eics_DTOP = std::tuple<>;
    using eocs_DTOP = std::tuple<cadmium::modeling::EOC<D3, D3_out, DTOP_out>>;
    using ics_DTOP = std::tuple<cadmium::modeling::IC<D1, D1_out, D3, D3_in>>;
    template<typename TIME>
    struct DTOP
            : public cadmium::modeling::pdevs::coupled_model<TIME, ips_DTOP, ops_DTOP, submodels_DTOP, eics_DTOP, eocs_DTOP, ics_DTOP> {
    };

    BOOST_AUTO_TEST_CASE(simple_outbox_cleanup_bug_test) {
        std::string expected_initial_state = "State for model cadmium::basic_models::pdevs::accumulator<int, float> is [0, 0]";
        std::string expected_accumulation_of_one = "State for model cadmium::basic_models::pdevs::accumulator<int, float> is [1, 0]";
        std::string accum_states = "State for model cadmium::basic_models::pdevs::accumulator";
        cadmium::engine::runner<float, DTOP, log_time_and_state> r{0.0};
        r.run_until(5.0);
        int count_initial_states = count_matches(expected_initial_state, oss.str());
        int count_expected_accumulation = count_matches(expected_accumulation_of_one, oss.str());
        int count_accum_states = count_matches(accum_states, oss.str());
        // The accumulator has to increment the count once only
        BOOST_CHECK_EQUAL(count_initial_states + count_expected_accumulation, count_accum_states);
    }

BOOST_AUTO_TEST_SUITE_END()

