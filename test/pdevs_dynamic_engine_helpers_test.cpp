/**
 * Copyright (c) 2017, Laouen Mayal Louan Belloli, Damian Vicino
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
#include <cadmium/basic_model/accumulator.hpp>
#include <cadmium/engine/pdevs_simulator.hpp>
#include <cadmium/engine/pdevs_coordinator.hpp>
#include <cadmium/basic_model/generator.hpp>
#include <cadmium/modeling/dynamic_models_helpers.hpp>
#include <typeindex>

/**
  * This test is for some common helper functions used by the dynamic simulator and coordinator classes
  */
BOOST_AUTO_TEST_SUITE( pdevs_dynamic_engine_helpers_test_suite )
    BOOST_AUTO_TEST_CASE(check_all_bags_empty_on_dynamic_empty_box_is_true){
//This test is suppose to pass only in CPP17 compilers, skipping in older compilers
#if __cplusplus > 201702
        //Define a tuple of bags (static version)
        struct test_in_0: public cadmium::in_port<int>{};
        struct test_in_1: public cadmium::in_port<double>{};
        using test_input_ports=std::tuple<test_in_0, test_in_1>;
        using input_bags=typename cadmium::make_message_bags<test_input_ports>::type;

        input_bags bs_tuple;

        //Create an empty "box" of bags (dynamic version)
        auto empty_box = cadmium::modeling::create_empty_dynamic_message_bags<input_bags>();
        
        //Check the created box has only empty bags
        BOOST_CHECK(cadmium::engine::dynamic_all_bags_empty<input_bags>(empty_box));
#else
        BOOST_WARN_MESSAGE(false, "Skippping check_all_bags_empty_on_dynamic_empty_box_is_true test because compiler is not C++17 compliant");
#endif
    }

    BOOST_AUTO_TEST_CASE(check_all_bags_empty_on_dynamic_non_empty_box_is_false){
//This test is suppose to pass only in CPP17 compilers, skipping in older compilers
#if __cplusplus > 201702
        //Create a box with bags with messages
        struct test_in_0: public cadmium::in_port<int>{};
        struct test_in_1: public cadmium::in_port<double>{};
        
        using test_input_ports=std::tuple<test_in_0, test_in_1>;
        using input_bags=typename cadmium::make_message_bags<test_input_ports>::type;
        
        input_bags bs_tuple;
        
        //Put some messages in the bags
        cadmium::get_messages<test_in_0>(bs_tuple).push_back(1);
        cadmium::get_messages<test_in_0>(bs_tuple).push_back(2);
        cadmium::get_messages<test_in_1>(bs_tuple).push_back(1.5);
        cadmium::get_messages<test_in_1>(bs_tuple).push_back(2.5);
        
        //add the messages from original box to a new dynamic one
        cadmium::dynamic_message_bags bs_map;
        cadmium::modeling::fill_map_from_bags(bs_tuple, bs_map);
        
        //check there is at least a bag that is not empty
        BOOST_CHECK(!cadmium::engine::dynamic_all_bags_empty<input_bags>(bs_map));
#else
        BOOST_WARN_MESSAGE(false, "Skippping check_all_bags_empty_on_dynamic_non_empty_box_is_false test because compiler is not C++17 compliant");
#endif
    }

BOOST_AUTO_TEST_SUITE_END()
