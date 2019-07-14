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
#include <cadmium/basic_model/pdevs/reset_generator_five_sec.hpp>
#include <cadmium/basic_model/pdevs/generator.hpp>
#include <cadmium/basic_model/pdevs/accumulator.hpp>
#include <cadmium/modeling/coupling.hpp>
#include <cadmium/engine/pdevs_coordinator.hpp>
#include <cadmium/engine/pdevs_simulator.hpp>
#include <cadmium/concept/concept_helpers.hpp>
#include <cadmium/engine/pdevs_engine_helpers.hpp>
#include <cadmium/logger/logger.hpp>


/**
  This test suite uses coordinators running simulators and basic models that were tested in other suites before
  The time for "next" in coordinators and simulators is absolute, starting at the time set
  by initializer.
  */

BOOST_AUTO_TEST_SUITE(pdevs_coordinator_test_suite)

    struct empty_coupled_model {
        using input_ports=std::tuple<>;
        using output_ports=std::tuple<>;
        using submodels = cadmium::modeling::models_tuple<>;
        using EICs = std::tuple<>;
        using EOCs = std::tuple<>;
        using ICs = std::tuple<>;
        template<typename TIME>
        using type=cadmium::modeling::pdevs::coupled_model<TIME, input_ports, output_ports, submodels, EICs, EOCs, ICs>;
    };

    BOOST_AUTO_TEST_CASE(pdevs_coordinator_empty_coupled_is_not_atomic_test) {
        BOOST_CHECK(!cadmium::concept::is_atomic<empty_coupled_model::type>::value());
    }

/**
 * p_coordinated_generator_test_suite tests that coordinating a coupled model with a generator
 * can be simulated properly
 */

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


    BOOST_AUTO_TEST_CASE(coordinated_generator_produces_right_output_test) {
        cadmium::engine::coordinator<coupled_generator, float, cadmium::logger::not_logger> cg;
        //check init sets the right next time
        cg.init(0);
        BOOST_CHECK_EQUAL(1.0f, cg.next());
        //collecting output before the next scheduled produces no output
        cg.collect_outputs(0.5f);
        auto output_bags = cg.outbox();
        BOOST_REQUIRE(cadmium::get_messages<coupled_out_port>(output_bags).empty());
        //check asking for output after next scheduled transition throws
        BOOST_CHECK_THROW(cg.collect_outputs(2.0f), std::domain_error);
        //check the right output is generated when asking at next time
        cg.collect_outputs(1.0f);
        output_bags = cg.outbox();
        BOOST_REQUIRE(!cadmium::engine::all_bags_empty(output_bags));
        BOOST_CHECK_EQUAL(cadmium::get_messages<coupled_out_port>(output_bags).size(), 1); //only a tick happened.

        //second cycle, all same checks one second later produce same results
        cg.advance_simulation(1.0f);
        //collecting output before the next scheduled produces no output
        cg.collect_outputs(1.5f);
        output_bags = cg.outbox();
        BOOST_REQUIRE(cadmium::get_messages<coupled_out_port>(output_bags).empty());
        //check asking for output after next scheduled transition throws
        BOOST_CHECK_THROW(cg.collect_outputs(3.0f), std::domain_error);
        //check the right output is generated when asking at next time
        cg.collect_outputs(2.0f);
        output_bags = cg.outbox();
        BOOST_CHECK_EQUAL(cadmium::get_messages<coupled_out_port>(output_bags).size(), 1); //only a tick happened.
    }

//2 generators connected to an infinite_counter are coordinated and routing messages correctly
//connecting generators to acumm coupled model definition

    template<typename TIME>
    using test_accumulator=cadmium::basic_models::pdevs::accumulator<int, TIME>;
    using test_accumulator_defs=cadmium::basic_models::pdevs::accumulator_defs<int>;
    using reset_tick=cadmium::basic_models::pdevs::accumulator_defs<int>::reset_tick;

    using g2a_iports = std::tuple<>;
    struct g2a_coupled_out_port : public cadmium::out_port<int> {
    };
    using g2a_oports = std::tuple<g2a_coupled_out_port>;
    using g2a_submodels=cadmium::modeling::models_tuple<test_accumulator, cadmium::basic_models::pdevs::reset_generator_five_sec, cadmium::basic_models::pdevs::int_generator_one_sec>;
    using g2a_eics=std::tuple<>;
    using g2a_eocs=std::tuple<
            cadmium::modeling::EOC<test_accumulator, test_accumulator_defs::sum, g2a_coupled_out_port>
    >;
    using g2a_ics=std::tuple<
            cadmium::modeling::IC<cadmium::basic_models::pdevs::int_generator_one_sec, cadmium::basic_models::pdevs::int_generator_one_sec_defs::out, test_accumulator, test_accumulator_defs::add>,
            cadmium::modeling::IC<cadmium::basic_models::pdevs::reset_generator_five_sec, cadmium::basic_models::pdevs::reset_generator_five_sec_defs::out, test_accumulator, test_accumulator_defs::reset>
    >;

    template<typename TIME>
    using coupled_g2a_model=cadmium::modeling::pdevs::coupled_model<TIME, g2a_iports, g2a_oports, g2a_submodels, g2a_eics, g2a_eocs, g2a_ics>;

    BOOST_AUTO_TEST_CASE(generators_send_to_accumulator_and_output_test) {
        cadmium::engine::coordinator<coupled_g2a_model, float, cadmium::logger::not_logger> cc;
        //check init sets the right next time
        cc.init(0);

        //first collection of output are empty
        BOOST_CHECK_EQUAL((float) 1, cc.next());

        cc.collect_outputs((float) 1);
        auto output_bags = cc.outbox();
        BOOST_REQUIRE(cadmium::get_messages<g2a_coupled_out_port>(output_bags).empty());
        cc.advance_simulation((float) 1);

        //next 3 collections of output are empty
        for (int i = 2; i < 6; i++) {
            BOOST_CHECK_EQUAL((float) i, cc.next());
            cc.collect_outputs((float) i);
            output_bags = cc.outbox();
            BOOST_REQUIRE(cadmium::get_messages<g2a_coupled_out_port>(output_bags).empty());
            cc.advance_simulation((float) i);
        }
        BOOST_CHECK_EQUAL((float) 5.0, cc.next()); //fifth advance triggers a reset and reschedules same time for next
        cc.collect_outputs(5.0f);
        output_bags = cc.outbox();

        BOOST_REQUIRE(!cadmium::engine::all_bags_empty(output_bags));
        BOOST_CHECK_EQUAL(cadmium::get_messages<g2a_coupled_out_port>(output_bags).size(), 1); //only a sum happened.
        BOOST_CHECK_EQUAL(cadmium::get_messages<g2a_coupled_out_port>(output_bags).at(0),
                          5); //5 ticks of 1 were counted
        cc.advance_simulation(5.0f);
        BOOST_CHECK_EQUAL(6.0f, cc.next());
        cc.collect_outputs(6.0f);
        output_bags = cc.outbox();
        BOOST_REQUIRE(cadmium::get_messages<g2a_coupled_out_port>(output_bags).empty());//was reset
    }

//next test is similar to previous, but models are split in 2 coupled ones
//2 generators connected to an infinite_counter are coordinated and routing messages correctly
//connecting generators to acumm coupled model definition

    template<typename TIME>
    using test_accumulator=cadmium::basic_models::pdevs::accumulator<int, TIME>;
    using test_accumulator_defs=cadmium::basic_models::pdevs::accumulator_defs<int>;
    using reset_tick=cadmium::basic_models::pdevs::accumulator_defs<int>::reset_tick;



    using empty_iports = std::tuple<>;
    using empty_eic=std::tuple<>;
    using empty_ic=std::tuple<>;

//2 generators doing output in 2 ports
    using generators_oports=std::tuple<cadmium::basic_models::pdevs::int_generator_one_sec_defs::out, cadmium::basic_models::pdevs::reset_generator_five_sec_defs::out>;
    using generators_submodels=cadmium::modeling::models_tuple<cadmium::basic_models::pdevs::reset_generator_five_sec, cadmium::basic_models::pdevs::int_generator_one_sec>;
    using generators_eoc=std::tuple<
            cadmium::modeling::EOC<cadmium::basic_models::pdevs::reset_generator_five_sec, cadmium::basic_models::pdevs::reset_generator_five_sec_defs::out, cadmium::basic_models::pdevs::reset_generator_five_sec_defs::out>,
            cadmium::modeling::EOC<cadmium::basic_models::pdevs::int_generator_one_sec, cadmium::basic_models::pdevs::int_generator_one_sec_defs::out, cadmium::basic_models::pdevs::int_generator_one_sec_defs::out>
    >;

    template<typename TIME>
    using coupled_generators_model=cadmium::modeling::pdevs::coupled_model<TIME, empty_iports, generators_oports, generators_submodels, empty_eic, generators_eoc, empty_ic>;

//1 accumulator wrapped in a coupled model
    using accumulator_eic=std::tuple<
            cadmium::modeling::EIC<test_accumulator_defs::add, test_accumulator, test_accumulator_defs::add>,
            cadmium::modeling::EIC<test_accumulator_defs::reset, test_accumulator, test_accumulator_defs::reset>
    >;
    using accumulator_eoc=std::tuple<
            cadmium::modeling::EOC<test_accumulator, test_accumulator_defs::sum, test_accumulator_defs::sum>
    >;

    using accumulator_submodels=cadmium::modeling::models_tuple<test_accumulator>;

    template<typename TIME>
    using coupled_accumulator_model=cadmium::modeling::pdevs::coupled_model<TIME, typename test_accumulator<TIME>::input_ports, typename test_accumulator<TIME>::output_ports, accumulator_submodels, accumulator_eic, accumulator_eoc, empty_ic>;


//top model interconnecting the 2 coupled models

    using top_outport = test_accumulator_defs::sum;
    using top_oports = std::tuple<top_outport>;
    using top_submodels=cadmium::modeling::models_tuple<coupled_generators_model, coupled_accumulator_model>;

    using top_eoc=std::tuple<
            cadmium::modeling::EOC<coupled_accumulator_model, test_accumulator_defs::sum, top_outport>
    >;
    using top_ic=std::tuple<
            cadmium::modeling::IC<coupled_generators_model, cadmium::basic_models::pdevs::int_generator_one_sec_defs::out, coupled_accumulator_model, test_accumulator_defs::add>,
            cadmium::modeling::IC<coupled_generators_model, cadmium::basic_models::pdevs::reset_generator_five_sec_defs::out, coupled_accumulator_model, test_accumulator_defs::reset>
    >;

    template<typename TIME>
    using top_model=cadmium::modeling::pdevs::coupled_model<TIME, empty_iports, top_oports, top_submodels, empty_eic, top_eoc, top_ic>;


    BOOST_AUTO_TEST_CASE(generators_send_to_accumulator_and_output_in_two_coupled_models_test) {
        cadmium::engine::coordinator<top_model, float, cadmium::logger::not_logger> cctop;
        //check init sets the right next time
        cctop.init(0);

        //first collection of output are empty
        BOOST_CHECK_EQUAL((float) 1, cctop.next());

        cctop.collect_outputs((float) 1);
        auto output_bags = cctop.outbox();
        BOOST_REQUIRE(cadmium::get_messages<top_outport>(output_bags).empty());
        cctop.advance_simulation((float) 1);

        //next 3 collections of output are empty
        for (int i = 2; i < 6; i++) {
            BOOST_CHECK_EQUAL((float) i, cctop.next());
            cctop.collect_outputs((float) i);
            output_bags = cctop.outbox();
            BOOST_REQUIRE(cadmium::get_messages<top_outport>(output_bags).empty());
            cctop.advance_simulation((float) i);
        }

        BOOST_CHECK_EQUAL((float) 5.0,
                          cctop.next()); //fifth advance triggers a reset and reschedules same time for next
        cctop.collect_outputs(5.0f);
        output_bags = cctop.outbox();

        BOOST_REQUIRE(!cadmium::engine::all_bags_empty(output_bags));
        BOOST_CHECK_EQUAL(cadmium::get_messages<top_outport>(output_bags).size(), 1); //only a sum happened.
        BOOST_CHECK_EQUAL(cadmium::get_messages<top_outport>(output_bags).at(0), 5); //5 ticks of 1 were counted
        cctop.advance_simulation(5.0f);
        BOOST_CHECK_EQUAL(6.0f, cctop.next());
        cctop.collect_outputs(6.0f);
        output_bags = cctop.outbox();
        BOOST_REQUIRE(cadmium::get_messages<top_outport>(output_bags).empty());//was reset

    }


BOOST_AUTO_TEST_SUITE_END()


