/**
 * Copyright (c) 2017, Laouen M. L. Belloli
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
#include <cadmium/basic_model/pdevs/accumulator.hpp>
#include <cadmium/engine/pdevs_simulator.hpp>
#include <cadmium/engine/pdevs_coordinator.hpp>
#include <cadmium/basic_model/pdevs/generator.hpp>
#include <cadmium/modeling/dynamic_models_helpers.hpp>
#include <typeindex>

/**
  * This test is for some common helper functions used by the dynamic atomic class
  */
BOOST_AUTO_TEST_SUITE( pdevs_dynamic_atomic_helpers_test_suite )

    BOOST_AUTO_TEST_CASE(fill_bags_from_map_test){

            struct test_in_0: public cadmium::in_port<int>{};
            struct test_in_1: public cadmium::in_port<double>{};

            cadmium::message_bag<test_in_0> bag_0;
            cadmium::message_bag<test_in_1> bag_1;

            bag_0.messages.push_back(1);
            bag_0.messages.push_back(2);
            bag_1.messages.push_back(1.5);
            bag_1.messages.push_back(2.5);

            std::map<std::type_index, boost::any> bs_map;
            bs_map[typeid(test_in_0)] = bag_0;
            bs_map[typeid(test_in_1)] = bag_1;

            using test_input_ports=std::tuple<test_in_0, test_in_1>;
            using input_bags=typename cadmium::make_message_bags<test_input_ports>::type;

            input_bags bs_tuple;
            cadmium::dynamic::modeling::fill_bags_from_map<input_bags>(bs_map, bs_tuple);
            BOOST_CHECK_EQUAL(bag_0.messages.size(), cadmium::get_messages<test_in_0>(bs_tuple).size());
            BOOST_CHECK_EQUAL(bag_1.messages.size(), cadmium::get_messages<test_in_1>(bs_tuple).size());
    }

    BOOST_AUTO_TEST_CASE(fill_map_from_bags_test){

            struct test_in_0: public cadmium::in_port<int>{};
            struct test_in_1: public cadmium::in_port<double>{};

            using test_input_ports=std::tuple<test_in_0, test_in_1>;
            using input_bags=typename cadmium::make_message_bags<test_input_ports>::type;

            using bag_0 = cadmium::message_bag<test_in_0>;
            using bag_1 = cadmium::message_bag<test_in_1>;

            input_bags bs_tuple;

            cadmium::get_messages<test_in_0>(bs_tuple).push_back(1);
            cadmium::get_messages<test_in_0>(bs_tuple).push_back(2);
            cadmium::get_messages<test_in_1>(bs_tuple).push_back(1.5);
            cadmium::get_messages<test_in_1>(bs_tuple).push_back(2.5);

            std::map<std::type_index, boost::any> bs_map;

            cadmium::dynamic::modeling::fill_map_from_bags<input_bags>(bs_tuple, bs_map);

            bag_0 tuple_bag_0 = std::get<0>(bs_tuple);
            bag_0 map_bag_0 = boost::any_cast<bag_0>(bs_map.at(typeid(test_in_0)));
            BOOST_CHECK_EQUAL(map_bag_0.messages.size(), tuple_bag_0.messages.size());

            bag_1 tuple_bag_1 = std::get<1>(bs_tuple);
            bag_1 map_bag_1 = boost::any_cast<bag_1>(bs_map.at(typeid(test_in_1)));
            BOOST_CHECK_EQUAL(map_bag_1.messages.size(), tuple_bag_1.messages.size());
    }


BOOST_AUTO_TEST_SUITE_END()
