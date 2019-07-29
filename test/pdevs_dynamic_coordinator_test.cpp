/**
 * Copyright (c) 2018-2019, Laouen M. L. Belloli, Damian Vicino
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
#include <cadmium/basic_model/pdevs/generator.hpp>
#include <cadmium/modeling/coupling.hpp>
#include <cadmium/modeling/dynamic_atomic.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>

BOOST_AUTO_TEST_SUITE(pdevs_dynamic_coordinator_test_suite)

    template<typename TIME>
    class custom_id_coupled : public cadmium::dynamic::modeling::coupled<TIME> {
    public:
        custom_id_coupled() : cadmium::dynamic::modeling::coupled<TIME>("custom_id_coupled") {};
    };

    BOOST_AUTO_TEST_CASE(empty_coupled_model_coordinator) {
        std::shared_ptr<cadmium::dynamic::modeling::coupled<float>> coupled = std::make_shared<custom_id_coupled<float>>();
        cadmium::dynamic::engine::coordinator<float, cadmium::logger::not_logger> c(coupled);
        BOOST_CHECK(coupled->get_id() == "custom_id_coupled");
        BOOST_CHECK(coupled->get_id() == c.get_model_id());
    }

    struct test_tick {
    };

    //generator for tick messages
    using out_port = cadmium::basic_models::pdevs::generator_defs<test_tick>::out;

    template<typename TIME>
    using test_tick_generator_base=cadmium::basic_models::pdevs::generator<test_tick, float>;

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

    struct coupled_out_port : public cadmium::out_port<test_tick> {
    };

    using iports = std::tuple<>;
    using oports = std::tuple<coupled_out_port>;

    using eocs=std::tuple<
            cadmium::modeling::EOC<dynamic_test_generator, out_port, coupled_out_port>
    >;
    using eics=std::tuple<>;
    using ics=std::tuple<>;

    BOOST_AUTO_TEST_CASE(coordinator_of_tic_coupled_model) {

        std::shared_ptr<cadmium::dynamic::modeling::model> sp_test_generator = cadmium::dynamic::translate::make_dynamic_atomic_model<test_generator, float>();

        cadmium::dynamic::translate::models_by_type models_by_type;
        models_by_type.emplace(typeid(dynamic_test_generator<float>), sp_test_generator);
        cadmium::dynamic::modeling::Ports input_ports = cadmium::dynamic::translate::make_ports<iports>();
        cadmium::dynamic::modeling::Ports output_ports = cadmium::dynamic::translate::make_ports<oports>();
        cadmium::dynamic::modeling::Models submodels = {sp_test_generator};
        cadmium::dynamic::modeling::EOCs dynamic_eocs = cadmium::dynamic::translate::make_dynamic_eoc<float, eocs>(
                models_by_type);
        cadmium::dynamic::modeling::EICs dynamic_eics = cadmium::dynamic::translate::make_dynamic_eic<float, eics>(
                models_by_type);
        cadmium::dynamic::modeling::ICs dynamic_ics = cadmium::dynamic::translate::make_dynamic_ic<float, ics>(
                models_by_type);

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
        //check init sets the right next time
        cg.init(0);
        BOOST_CHECK_EQUAL(1.0f, cg.next());
        //collecting output before the next scheduled produces no output
        cg.collect_outputs(0.5f);
        auto output_bags = cg.outbox();
        BOOST_REQUIRE(output_bags.empty());
        //check asking for output after next scheduled transition throws
        BOOST_CHECK_THROW(cg.collect_outputs(2.0f), std::domain_error);
        //check the right output is generated when asking at next time
        cg.collect_outputs(1.0f);
        output_bags = cg.outbox();
        BOOST_REQUIRE(!output_bags.empty());
        BOOST_CHECK_EQUAL(output_bags.size(), 1);
        BOOST_REQUIRE(output_bags.find(typeid(coupled_out_port)) != output_bags.end());
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<coupled_out_port>>(
                output_bags.at(typeid(coupled_out_port))).messages.size(), 1); //only a tick happened.

        //second cycle, all same checks one second later produce same results
        cg.advance_simulation(1.0f);
        //collecting output before the next scheduled produces no output
        cg.collect_outputs(1.5f);
        output_bags = cg.outbox();
        BOOST_REQUIRE(output_bags.empty());
        //check asking for output after next scheduled transition throws
        BOOST_CHECK_THROW(cg.collect_outputs(3.0f), std::domain_error);
        //check the right output is generated when asking at next time
        cg.collect_outputs(2.0f);
        output_bags = cg.outbox();
        BOOST_REQUIRE(!output_bags.empty());
        BOOST_CHECK_EQUAL(output_bags.size(), 1);
        BOOST_REQUIRE(output_bags.find(typeid(coupled_out_port)) != output_bags.end());
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<coupled_out_port>>(
                output_bags.at(typeid(coupled_out_port))).messages.size(), 1); //only a tick happened.
    }

BOOST_AUTO_TEST_SUITE_END()