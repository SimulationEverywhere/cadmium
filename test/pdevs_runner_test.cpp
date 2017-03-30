/**
 * Copyright (c) 2013-2017, Damian Vicino
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
#include <cadmium/basic_model/generator.hpp>
#include <cadmium/modeling/coupled_model.hpp>
#include <cadmium/engine/pdevs_runner.hpp>

/**
  This test suite is running basic models that were tested in other suites before
  The time for "next" in runner is absolute, starting at the time set by init_time.
  */

BOOST_AUTO_TEST_SUITE( pdevs_runner_test_suite )

//generator in a coupled model definition pieces
//message representing ticks
struct test_tick{

};

//generator for tick messages
using out_port = cadmium::basic_models::generator_defs<test_tick>::out;
template <typename TIME>
using test_tick_generator_base=cadmium::basic_models::generator<test_tick, TIME>;

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
struct coupled_out_port : public cadmium::out_port<test_tick>{};
using oports = std::tuple<coupled_out_port>;
using submodels=cadmium::modeling::models_tuple<test_generator>;
using eics=std::tuple<>;
using eocs=std::tuple<
    cadmium::modeling::EOC<test_generator, out_port, coupled_out_port>
>;
using ics=std::tuple<>;

template<typename TIME>
using coupled_generator=cadmium::modeling::coupled_model<TIME, iports, oports, submodels, eics, eocs, ics>;

BOOST_AUTO_TEST_SUITE( pdevs_silent_runner_test_suite )

BOOST_AUTO_TEST_CASE( pdevs_runner_of_a_generator_in_a_coupled_for_a_minute_test){
    cadmium::engine::runner<float, coupled_generator> r{0.0};
    float next_to_end_time = r.runUntil(60.0);
    BOOST_CHECK_EQUAL(60.0, next_to_end_time);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( loggers_sources_runner_test_suite )

namespace {
    std::ostringstream oss;

    struct oss_test_sink_provider{
        static std::ostream& sink(){
            return oss;
        }
    };
}


BOOST_AUTO_TEST_CASE( runner_logs_global_time_advances_test )
{
    oss.clear();
    //logger definition
    using log_gt_to_oss=cadmium::logger::logger<cadmium::logger::logger_global_time, cadmium::logger::verbatim_formater, oss_test_sink_provider>;

    //setup runner
    cadmium::engine::runner<float, coupled_generator, log_gt_to_oss> r{0.0};
    float next_to_end_time = r.runUntil(3.0);

    //check the string
    BOOST_CHECK_EQUAL(oss.str(), "0\n1\n2\n");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

