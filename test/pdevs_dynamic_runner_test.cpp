/**
 * Copyright (c) 2018-2019, Laouen M. L. Belloli, Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis, Universidad de Buenos Aires
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

#include <cadmium/basic_model/pdevs/generator.hpp>

#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_atomic.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>

#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/modeling/coupling.hpp>

BOOST_AUTO_TEST_SUITE(pdevs_dynamic_runner_test_suite)

    //generator in a coupled model definition pieces
    //message representing ticks
    struct test_tick {
    };

    //generator for tick messages
    using out_port = cadmium::basic_models::pdevs::generator_defs<test_tick>::out;
    template<typename TIME>
    using test_tick_generator_base=cadmium::basic_models::pdevs::generator<test_tick, TIME>;

    template<typename TIME>
    struct test_generator : public test_tick_generator_base<TIME> {
        float period() const override {
            return 1.0f; //using float for time in this test, ticking every second
        }

        test_tick output_message() const override {
            return test_tick();
        }
    };

    //generator of ticks coupled model definition
    using iports = std::tuple<>;
    struct coupled_out_port : public cadmium::out_port<test_tick> {
    };
    using oports = std::tuple<coupled_out_port>;
    using submodels=cadmium::modeling::models_tuple<test_generator>;
    using eics=std::tuple<>;
    using eocs=std::tuple<
            cadmium::modeling::EOC<test_generator, out_port, coupled_out_port>
    >;
    using ics=std::tuple<>;

    template<typename TIME>
    using coupled_generator=cadmium::modeling::pdevs::coupled_model<TIME, iports, oports, submodels, eics, eocs, ics>;

    // std::shared_ptr<cadmium::dynamic::modeling::coupled<float>>
    auto coupled = cadmium::dynamic::translate::make_dynamic_coupled_model<float, coupled_generator>();

    // used to get the model id
    std::shared_ptr<cadmium::dynamic::modeling::model> sp_test_generator = cadmium::dynamic::translate::make_dynamic_atomic_model<test_generator, float>();

    BOOST_AUTO_TEST_SUITE(pdevs_silent_dynamic_runner_test_suite)

        BOOST_AUTO_TEST_CASE(pdevs_dynamic_runner_of_a_generator_in_a_coupled_for_a_minute_test) {
            cadmium::dynamic::engine::runner<float, cadmium::logger::not_logger> r(coupled, 0.0);
            float next_to_end_time = r.run_until(60.0);
            BOOST_CHECK_EQUAL(60.0, next_to_end_time);
        }

    BOOST_AUTO_TEST_SUITE_END()

    BOOST_AUTO_TEST_SUITE(loggers_sources_dynamic_runner_test_suite)

        namespace {
            std::ostringstream oss;

            struct oss_test_sink_provider {
                static std::ostream &sink() {
                    return oss;
                }
            };
        }


        BOOST_AUTO_TEST_CASE(dynamic_runner_logs_global_time_advances_test) {
            oss.str("");
            //logger definition
            using log_gt_to_oss=cadmium::logger::logger<cadmium::logger::logger_global_time, cadmium::dynamic::logger::formatter<float>, oss_test_sink_provider>;

            //setup runner
            cadmium::dynamic::engine::runner<float, log_gt_to_oss> r(coupled, 0.0);
            r.run_until(3.0);

            //check the string
            auto expected = "0\n"  //runnner init time
                            "1\n"  //runner first advance
                            "2\n"; //runner last advance
            BOOST_CHECK_EQUAL(oss.str(), expected);
        }

        BOOST_AUTO_TEST_CASE(dynamic_simulation_logs_info_on_setup_and_start_loops_and_end_of_run_test) {
            //This test integrates log output from runner, coordinator and simulator.
            oss.str("");
            //logger definition
            using log_info_to_oss=cadmium::logger::logger<cadmium::logger::logger_info, cadmium::dynamic::logger::formatter<float>, oss_test_sink_provider>;

            //setup runner
            cadmium::dynamic::engine::runner<float, log_info_to_oss> r(coupled, 0.0);
            r.run_until(2.0);

            //check the string
            std::ostringstream expected_oss;
            expected_oss << "Preparing model\n"; //setup of model by runner

            //top model is init
            expected_oss << "Coordinator for model ";
            expected_oss << coupled->get_id();
            expected_oss << " initialized to time 0\n";

            //generator model is init
            expected_oss << "Simulator for model ";
            expected_oss << sp_test_generator->get_id();
            expected_oss << " initialized to time 0\n";


            expected_oss << "Starting run\n"; //starting simulation main loop in runner

            //top model collects outputs
            expected_oss << "Coordinator for model ";
            expected_oss << coupled->get_id();
            expected_oss << " collecting output at time 1\n";

            //generator model collects outputs
            expected_oss << "Simulator for model ";
            expected_oss << sp_test_generator->get_id();
            expected_oss << " collecting output at time 1\n";

            //top model advances simulation
            expected_oss << "Coordinator for model ";
            expected_oss << coupled->get_id();
            expected_oss << " advancing simulation from time 0 to 1\n";

            //generator model advances simulation
            expected_oss << "Simulator for model ";
            expected_oss << sp_test_generator->get_id();
            expected_oss << " advancing simulation from time 0 to 1\n";

            expected_oss << "Finished run\n"; //finished simulation and exiting runner
            BOOST_CHECK_EQUAL(oss.str(), expected_oss.str());
        }

        BOOST_AUTO_TEST_CASE(dynamic_simulation_logs_state_only_show_state_changes_and_initial_state_test) {
            //This test integrates log output from runner, coordinator and simulator.
            oss.str("");
            //logger definition
            using log_info_to_oss=cadmium::logger::logger<cadmium::logger::logger_state, cadmium::dynamic::logger::formatter<float>, oss_test_sink_provider>;

            //setup runner
            cadmium::dynamic::engine::runner<float, log_info_to_oss> r(coupled, 0.0);
            r.run_until(3.0);

            //check the string
            std::ostringstream expected_oss;
            for (int i = 0; i < 3; i++) {// initial state and 2 more states
                expected_oss << "State for model ";
                expected_oss << sp_test_generator->get_id();
                expected_oss << " is 0\n";
            }

            BOOST_CHECK_EQUAL(oss.str(), expected_oss.str());
        }

        BOOST_AUTO_TEST_CASE(dynamic_simulation_logs_messages_generated_in_atomic_models_test) {
            //This test integrates log output from runner, coordinator and simulator.
            oss.str("");
            //logger definition
            using log_info_to_oss=cadmium::logger::logger<cadmium::logger::logger_messages, cadmium::dynamic::logger::formatter<float>, oss_test_sink_provider>;

            //setup runner
            cadmium::dynamic::engine::runner<float, log_info_to_oss> r(coupled, 0.0);
            r.run_until(2.0);

            //check the string
            std::ostringstream expected_oss;
            expected_oss << "[";
            expected_oss << boost::typeindex::type_id<out_port>().pretty_name();
            expected_oss << ": {obscure message of type ";
            expected_oss << boost::typeindex::type_id<test_tick>().pretty_name();
            expected_oss << "}] generated by model ";
            expected_oss << sp_test_generator->get_id();
            expected_oss << "\n";

            BOOST_CHECK_EQUAL(oss.str(), expected_oss.str());
        }

        BOOST_AUTO_TEST_CASE(dynamic_simulation_logs_local_time_in_simulators_test) {
            //This test integrates log output from runner, coordinator and simulator.
            oss.str("");
            //logger definition
            using log_info_to_oss=cadmium::logger::logger<cadmium::logger::logger_local_time, cadmium::dynamic::logger::formatter<float>, oss_test_sink_provider>;

            //setup runner
            cadmium::dynamic::engine::runner<float, log_info_to_oss> r(coupled, 0.0);
            r.run_until(2.0);

            //check the string
            std::ostringstream expected_oss;
            expected_oss << "Elapsed in model ";
            expected_oss << sp_test_generator->get_id();
            expected_oss << " is 1s\n";
            BOOST_CHECK_EQUAL(oss.str(), expected_oss.str());
        }

        BOOST_AUTO_TEST_CASE(dynamic_simulation_logs_routing_of_eoc_in_coordinator_test) {
            //This test integrates log output from runner, coordinator and simulator.
            oss.str("");
            //logger definition
            using log_info_to_oss=cadmium::logger::logger<cadmium::logger::logger_message_routing, cadmium::dynamic::logger::formatter<float>, oss_test_sink_provider>;

            //setup runner
            cadmium::dynamic::engine::runner<float, log_info_to_oss> r(coupled, 0.0);
            r.run_until(2.0);

            //check the string
            std::ostringstream expected_oss;
            //EOC of one event
            expected_oss << "EOC for model ";
            expected_oss << coupled->get_id();
            expected_oss << "\n in port ";
            expected_oss << boost::typeindex::type_id<coupled_out_port>().pretty_name();
            expected_oss << " has {obscure message of type ";
            expected_oss << boost::typeindex::type_id<test_tick>().pretty_name();
            expected_oss << "} routed from ";
            expected_oss << boost::typeindex::type_id<out_port>().pretty_name();
            expected_oss << " with messages {obscure message of type ";
            expected_oss << boost::typeindex::type_id<test_tick>().pretty_name();
            expected_oss << "}\n";
            //empty IC
            expected_oss << "IC for model ";
            expected_oss << coupled->get_id();
            expected_oss << "\n";
            //empty EIC
            expected_oss << "EIC for model ";
            expected_oss << coupled->get_id();
            expected_oss << "\n";

            BOOST_CHECK_EQUAL(oss.str(), expected_oss.str());
        }

    BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

