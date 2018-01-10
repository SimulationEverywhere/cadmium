/**
 * Copyright (c) 2018, Laouen M. L. Belloli, Damian Vicino
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
#include <cadmium/basic_model/generator.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/engine/pdevs_dynamic_coordinator.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/modeling/coupled_model.hpp>
#include <cadmium/logger/common_loggers.hpp>

BOOST_AUTO_TEST_SUITE( pdevs_dynamic_runner_test_suite )

    //generator in a coupled model definition pieces
    //message representing ticks
    struct test_tick{};

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

    template<typename TIME>
    using dynamic_test_generator = cadmium::dynamic::modeling::atomic<test_generator, TIME>;
    //generator of ticks coupled model definition

    using iports = std::tuple<>;
    struct coupled_out_port : public cadmium::out_port<test_tick>{};
    using oports = std::tuple<coupled_out_port>;
    using eics=std::tuple<>;
    using eocs=std::tuple<
            cadmium::modeling::EOC<dynamic_test_generator, out_port, coupled_out_port>
    >;
    using ics=std::tuple<>;

    std::shared_ptr<cadmium::dynamic::modeling::model> sp_test_generator = cadmium::dynamic::modeling::make_atomic_model<test_generator, float>();
    cadmium::dynamic::modeling::Ports input_ports = cadmium::dynamic::translate::make_ports<iports>();
    cadmium::dynamic::modeling::Ports output_ports = cadmium::dynamic::translate::make_ports<oports>();
    cadmium::dynamic::modeling::Models submodels = {sp_test_generator};
    cadmium::dynamic::modeling::EOCs dynamic_eocs = cadmium::dynamic::translate::make_dynamic_eoc<float, eocs>();
    cadmium::dynamic::modeling::EICs dynamic_eics = cadmium::dynamic::translate::make_dynamic_eic<float, eics>();
    cadmium::dynamic::modeling::ICs dynamic_ics = cadmium::dynamic::translate::make_dynamic_ic<float, ics>();

    std::shared_ptr<cadmium::dynamic::modeling::coupled<float>> coupled = std::make_shared<cadmium::dynamic::modeling::coupled<float>>(
            "dynamic_coupled_test_generator",
            submodels,
            input_ports,
            output_ports,
            dynamic_eics,
            dynamic_eocs,
            dynamic_ics
    );

    cadmium::dynamic::engine::coordinator<float, cadmium::logger::not_logger> cg(coupled);

    BOOST_AUTO_TEST_SUITE( pdevs_silent_runner_test_suite )

        BOOST_AUTO_TEST_CASE( pdevs_dynamic_runner_of_a_generator_in_a_coupled_for_a_minute_test){
            cadmium::dynamic::engine::runner<float, cadmium::logger::not_logger> r(coupled, 0.0);
            float next_to_end_time = r.runUntil(60.0);
            BOOST_CHECK_EQUAL(60.0, next_to_end_time);
        }

    BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

