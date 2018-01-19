/**
 * Copyright (c) 2018, Laouen M. Louan Belloli
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

#include <iostream>

#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_models_helpers.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>


BOOST_AUTO_TEST_SUITE( test_links )

    BOOST_AUTO_TEST_CASE( test_link_creation) {
        struct test_out: public cadmium::out_port<int>{};
        struct test_in: public cadmium::in_port<int>{};

        std::shared_ptr<cadmium::dynamic::engine::link_abstract> link_test = cadmium::dynamic::translate::make_link<test_out, test_in>();
        BOOST_CHECK(link_test->from_port_type_index() == typeid(test_out));
        BOOST_CHECK(link_test->to_port_type_index() == typeid(test_in));
    }

    BOOST_AUTO_TEST_CASE( test_passing_message_between_bags_does_not_modify_from_bag_and_copies_messages_to_to_bag ) {
        struct test_out: public cadmium::out_port<int>{};
        struct test_in: public cadmium::in_port<int>{};

        std::shared_ptr<cadmium::dynamic::engine::link_abstract> link_test = cadmium::dynamic::translate::make_link<test_out, test_in>();

        cadmium::message_bag<test_out> bag_out;

        bag_out.messages.push_back(3);
        cadmium::dynamic::message_bags bag_from;
        bag_from[link_test->from_port_type_index()] = bag_out;

        BOOST_CHECK_EQUAL(bag_out.messages.size(), 1);
        BOOST_CHECK_EQUAL(bag_out.messages.front(), 3);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_out>>(bag_from.at(link_test->from_port_type_index())).messages.size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_out>>(bag_from.at(link_test->from_port_type_index())).messages.front(), 3);

        cadmium::dynamic::message_bags bag_to;
        link_test->route_messages(bag_from, bag_to);

        // bag_from was not modified
        BOOST_CHECK_EQUAL(bag_out.messages.size(), 1);
        BOOST_CHECK_EQUAL(bag_out.messages.front(), 3);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_out>>(bag_from.at(link_test->from_port_type_index())).messages.size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_out>>(bag_from.at(link_test->from_port_type_index())).messages.front(), 3);

        // bag_to has the new message
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_in>>(bag_to.at(link_test->to_port_type_index())).messages.size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_in>>(bag_to.at(link_test->to_port_type_index())).messages.front(), 3);

        // testing the pass_messages for defined bags
        link_test->route_messages(bag_from, bag_to);

        // bag_from was not modified
        BOOST_CHECK_EQUAL(bag_out.messages.size(), 1);
        BOOST_CHECK_EQUAL(bag_out.messages.front(), 3);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_out>>(bag_from.at(link_test->from_port_type_index())).messages.size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_out>>(bag_from.at(link_test->from_port_type_index())).messages.front(), 3);

        // to_bag has one more new message
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_in>>(bag_to.at(link_test->to_port_type_index())).messages.size(), 2);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_in>>(bag_to.at(link_test->to_port_type_index())).messages[0], 3);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_in>>(bag_to.at(link_test->to_port_type_index())).messages[1], 3);
    }

    BOOST_AUTO_TEST_CASE( make_ports_from_cadmium_tuple_port_type ) {
        struct in_port_0 : public cadmium::in_port<int>{};
        struct in_port_1 : public cadmium::in_port<int>{};
        using iports = std::tuple<in_port_0,in_port_1>;
        struct out_port_0 : public cadmium::out_port<int>{};
        using oports = std::tuple<out_port_0>;

        cadmium::dynamic::modeling::Ports input_ports = cadmium::dynamic::translate::make_ports<iports>();
        cadmium::dynamic::modeling::Ports output_ports = cadmium::dynamic::translate::make_ports<oports>();

        BOOST_CHECK_EQUAL(input_ports.size(), 2);
        BOOST_CHECK_EQUAL(output_ports.size(), 1);

        std::type_index type_index_in_port_0 = typeid(in_port_0);
        std::type_index type_index_in_port_1 = typeid(in_port_1);
        std::type_index type_index_out_port_0 = typeid(out_port_0);
        BOOST_CHECK(type_index_in_port_0 == input_ports[0]);
        BOOST_CHECK(type_index_in_port_1 == input_ports[1]);
        BOOST_CHECK(type_index_out_port_0 == output_ports[0]);
    }

    BOOST_AUTO_TEST_CASE( make_eic_from_cadmium_tuple_eic_type ) {
        //TODO(Lao): implement this test
    }

BOOST_AUTO_TEST_SUITE_END()