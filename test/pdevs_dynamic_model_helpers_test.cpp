/**
 * Copyright (c) 2017, Laouen Mayal Louan Belloli
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
#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/modeling/ports.hpp>
#include <iostream>


BOOST_AUTO_TEST_SUITE( test_links )

    BOOST_AUTO_TEST_CASE( test_link_creation) {
        struct test_out: public cadmium::out_port<int>{};
        struct test_in: public cadmium::in_port<int>{};

        std::shared_ptr<cadmium::dynamic::link_abstract> link_test = cadmium::dynamic::make_link<test_out, test_in>();
        BOOST_CHECK(link_test->from_type_index() == typeid(cadmium::message_bag<test_out>));
        BOOST_CHECK(link_test->to_type_index() == typeid(cadmium::message_bag<test_in>));
    }

    BOOST_AUTO_TEST_CASE( test_passing_message_between_bags ) {
        struct test_out: public cadmium::out_port<int>{};
        struct test_in: public cadmium::in_port<int>{};

        std::shared_ptr<cadmium::dynamic::link_abstract> link_test = cadmium::dynamic::make_link<test_out, test_in>();

        cadmium::message_bag<test_out> bag_out;

        bag_out.messages.push_back(3);
        boost::any dynamic_bag_out = bag_out;

        BOOST_CHECK_EQUAL(bag_out.messages.size(), 1);
        BOOST_CHECK_EQUAL(bag_out.messages.front(), 3);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_out>>(dynamic_bag_out).messages.size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_out>>(dynamic_bag_out).messages.front(), 3);

        boost::any dynamic_bag_in = link_test->pass_messages_to_new_bag(dynamic_bag_out);

        // from_bag was not modified
        BOOST_CHECK_EQUAL(bag_out.messages.size(), 1);
        BOOST_CHECK_EQUAL(bag_out.messages.front(), 3);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_out>>(dynamic_bag_out).messages.size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_out>>(dynamic_bag_out).messages.front(), 3);

        // to_bag has the new message
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_in>>(dynamic_bag_in).messages.size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_in>>(dynamic_bag_in).messages.front(), 3);

        // testing the pass_messages for defined bags
        link_test->pass_messages(dynamic_bag_out, dynamic_bag_in);

        // from_bag was not modified
        BOOST_CHECK_EQUAL(bag_out.messages.size(), 1);
        BOOST_CHECK_EQUAL(bag_out.messages.front(), 3);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_out>>(dynamic_bag_out).messages.size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_out>>(dynamic_bag_out).messages.front(), 3);

        // to_bag has the one more new message
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_in>>(dynamic_bag_in).messages.size(), 2);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_in>>(dynamic_bag_in).messages[0], 3);
        BOOST_CHECK_EQUAL(boost::any_cast<cadmium::message_bag<test_in>>(dynamic_bag_in).messages[1], 3);
    }

BOOST_AUTO_TEST_SUITE_END()