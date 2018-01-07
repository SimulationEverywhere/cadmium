/**
 * Copyright (c) 2018, Laouen M. L. Belloli
 * Carleton University, Universidad de Buenos Aires
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
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/engine/pdevs_dynamic_coordinator.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/basic_model/generator.hpp>
#include <cadmium/modeling/coupled_model.hpp>
#include <cadmium/modeling/dynamic_atomic.hpp>

BOOST_AUTO_TEST_SUITE( pdevs_coordinator_test_suite )

    template <typename TIME>
    class custom_id_coupled : public cadmium::dynamic::modeling::coupled<TIME> {
    public:
        custom_id_coupled() : cadmium::dynamic::modeling::coupled<TIME>("custom_id_coupled") {};
    };

    BOOST_AUTO_TEST_CASE( empty_coupled_model_coordinator ) {
        std::shared_ptr<cadmium::dynamic::modeling::coupled<float>> coupled = std::make_shared<custom_id_coupled<float>>();
        cadmium::dynamic::engine::coordinator<float, cadmium::logger::not_logger> c(coupled);
        BOOST_CHECK_EQUAL(coupled->get_id(), "custom_id_coupled");
        BOOST_CHECK_EQUAL(coupled->get_id(), c.get_model_id());
    }

    struct test_tick {};

    //generator for tick messages
    using out_port = cadmium::basic_models::generator_defs<test_tick>::out;

    template <typename TIME>
    using test_tick_generator_base=cadmium::basic_models::generator<test_tick, float>;

    template<typename TIME>
    struct test_generator : public test_tick_generator_base<TIME> {
        float period() const override {
            return 1.0f; //using float for time in this test, ticking every second
        }
        test_tick output_message() const override {
            return test_tick();
        }
    };

    struct coupled_out_port : public cadmium::out_port<test_tick>{};

    using iports = std::tuple<>;
    using oports = std::tuple<coupled_out_port>;

    using eocs=std::tuple<
            cadmium::modeling::EOC<test_generator, out_port, coupled_out_port>
    >;

    BOOST_AUTO_TEST_CASE( coordinator_of_tic_coupled_model ) {

        std::shared_ptr<cadmium::dynamic::modeling::model> sp_test_generator = cadmium::dynamic::modeling::make_atomic_model<test_generator, float>();

        cadmium::dynamic::modeling::Ports input_ports = cadmium::dynamic::modeling::make_ports<iports>();
        cadmium::dynamic::modeling::Ports output_ports = cadmium::dynamic::modeling::make_ports<oports>();
        cadmium::dynamic::modeling::Models submodels = {sp_test_generator};

        //TODO(Lao): finish this test

//        using ics=std::tuple<>;
//        using eics=std::tuple<>;
//
//        template<typename TIME>
//        using coupled_generator=cadmium::modeling::coupled_model<TIME, iports, oports, submodels, eics, eocs, ics>;
    }

BOOST_AUTO_TEST_SUITE_END()