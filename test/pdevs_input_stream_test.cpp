/**
 * Copyright (c) 2013-2015, 
 * Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis
 * Cristina Ruiz Martín
 * Carleton University, Universidad de Valladolid
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
#include <cadmium/basic_model/input_stream.hpp>
#include <cmath>
#include <boost/any.hpp>


using namespace std;

BOOST_AUTO_TEST_SUITE( pdevs_basic_models_suite )
BOOST_AUTO_TEST_SUITE( pistream_test_suite )

using input_stream = cadmium::basic_models::input_stream<float, boost::any, int, int>;
using Time = float;
BOOST_AUTO_TEST_CASE( pistream_simple_of_single_events_test )
{
    //Create a input_stream with events every second outputing [1..10]
    //Check that outputs the 10 numbers every 1 second and last one is infinity

    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("0 0");
    //init
    input_stream pf{piss, Time(0)};
    BOOST_CHECK_EQUAL(pf.time_advance(), Time(0));

    // only message
    auto outmb1 = pf.output();
    BOOST_REQUIRE_EQUAL(cadmium::get_messages<typename input_stream::out>(outmb1).size(), 1);
    BOOST_CHECK_EQUAL(boost::any_cast<int>(cadmium::get_messages<typename input_stream::out>(outmb1)[0]), 0);
    pf.internal_transition();
    BOOST_CHECK( isinf(pf.time_advance()));
}

BOOST_AUTO_TEST_CASE( pistream_simple_of_multiple_events_test_3 )
{
    //Create a input_stream with events every second outputing [1..10]
    //Check that outputs the 10 numbers every 1 second and last one is infinity

    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("0 0 \n 0 1 \n 0 2 ");
    //init
    input_stream pf{piss, Time(0)};
    BOOST_CHECK_EQUAL(pf.time_advance(), Time(0));

    // only output
    auto outmb1 = pf.output();
    BOOST_REQUIRE_EQUAL(cadmium::get_messages<typename input_stream::out>(outmb1).size(), 3);
    pf.internal_transition();
    BOOST_CHECK( isinf(pf.time_advance()));
}

BOOST_AUTO_TEST_CASE( pistream_simple_of_multiple_events_test_2 )
{
    //Create a input_stream with events every second outputing [1..10]
    //Check that outputs the 10 numbers every 1 second and last one is infinity

    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("0 0 \n 0 1");
    //init
    input_stream pf{piss, Time(0)};
    BOOST_CHECK_EQUAL(pf.time_advance(), Time(0));

    // only output
    auto outmb1 = pf.output();
    BOOST_REQUIRE_EQUAL(cadmium::get_messages<typename input_stream::out>(outmb1).size(), 2);
    pf.internal_transition();
    BOOST_CHECK( isinf(pf.time_advance()));
}


BOOST_AUTO_TEST_CASE( pistream_as_generator_of_single_events_test )
{
    //Create a input_stream with events every second outputing [1..10]
    //Check that outputs the 10 numbers every 1 second and last one is infinity

    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("0 0 \n 1 1 \n 2 2 \n 3 3 \n 4 4 \n 5 5 \n 6 6 \n 7 7 \n 8 8 \n 9 9 \n 10 10");
    //init
    input_stream pf{piss, Time(0)};
    BOOST_CHECK_EQUAL(pf.time_advance(), Time(0));

    //consume
    for (int i=0; i < 10 ; i++){
        auto outmb1 = pf.output();
        BOOST_REQUIRE_EQUAL(cadmium::get_messages<typename input_stream::out>(outmb1).size(), 1);
        BOOST_CHECK_EQUAL(boost::any_cast<int>(cadmium::get_messages<typename input_stream::out>(outmb1)[0]), i);
        pf.internal_transition();
        BOOST_CHECK_EQUAL(pf.time_advance(), Time(1));
    }
    //last message
    auto outmb2 = pf.output();
    BOOST_REQUIRE_EQUAL(cadmium::get_messages<typename input_stream::out>(outmb2).size(), 1);
    BOOST_CHECK_EQUAL(boost::any_cast<int>(cadmium::get_messages<typename input_stream::out>(outmb2)[0]), 10);
    pf.internal_transition();
    BOOST_CHECK( isinf(pf.time_advance()));
}


BOOST_AUTO_TEST_CASE( pistream_as_generator_of_multiple_events_test )
{
    //Create a input_stream with events every second outputing [1..10]
    //Check that outputs the 10 numbers every 1 second and last one is infinity

    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("1 1 \n 1 1 \n 2 2 \n 2 2 \n 3 3 \n 3 3 \n 4 4 \n 4 4 \n 5 5 \n 5 5");
    //init
    input_stream pf{piss, Time(0)};
    BOOST_CHECK_EQUAL(pf.time_advance(), Time(1));
    //time_advance simulation
    for (int i=1; i < 5; i++){
        auto outmb1 = pf.output();
        BOOST_REQUIRE_EQUAL(cadmium::get_messages<typename input_stream::out>(outmb1).size(), 2);
        BOOST_CHECK_EQUAL(boost::any_cast<int>(cadmium::get_messages<typename input_stream::out>(outmb1)[0]), i);
        BOOST_CHECK_EQUAL(boost::any_cast<int>(cadmium::get_messages<typename input_stream::out>(outmb1)[1]), i);
        pf.internal_transition();
        BOOST_CHECK_EQUAL(pf.time_advance(), Time(1));
    }
    //last item
    auto outmb2 = pf.output();
    BOOST_REQUIRE_EQUAL(cadmium::get_messages<typename input_stream::out>(outmb2).size(), 2);
    BOOST_CHECK_EQUAL(boost::any_cast<int>(cadmium::get_messages<typename input_stream::out>(outmb2)[0]), 5);
    BOOST_CHECK_EQUAL(boost::any_cast<int>(cadmium::get_messages<typename input_stream::out>(outmb2)[1]), 5);
    pf.internal_transition();
    BOOST_CHECK( isinf(pf.time_advance()));
}

//custom processor of input
BOOST_AUTO_TEST_CASE( pistream_with_custom_processor_as_generator_of_multiple_events_test )
{
    //Create a input_stream with events every second outputing [1..10]
    //Check that outputs the 10 numbers every 1 second and last one is infinity

    shared_ptr<istringstream> piss{ new istringstream{} };
    piss->str("1 hello \n 1 world \n 2 hello \n 2 world");
    //init
    input_stream pf{piss, Time(0),
                [](const string& s, Time& t_next, boost::any& m_next)->void{
            //intermediary vars for casting
            int tmp_next;
            string tmp_next_out;
            std::stringstream ss;
            ss.str(s);
            ss >> tmp_next;
            t_next = static_cast<Time>(tmp_next);
            ss >> tmp_next_out;
            m_next = static_cast<boost::any>(tmp_next_out);
            std::string thrash;
            ss >> thrash;
            if ( 0 != thrash.size()) throw std::exception();
        }};

    BOOST_CHECK_EQUAL(pf.time_advance(), Time(1));
    //time_advance simulation
    auto outmb1 = pf.output();
    BOOST_REQUIRE_EQUAL(cadmium::get_messages<typename input_stream::out>(outmb1).size(), 2);
    BOOST_CHECK(any_of(cadmium::get_messages<typename input_stream::out>(outmb1).begin(), cadmium::get_messages<typename input_stream::out>(outmb1).end(), [](const boost::any& m ){ string s = boost::any_cast<string>(m); return s.compare("hello")==0;}));
    BOOST_CHECK(any_of(cadmium::get_messages<typename input_stream::out>(outmb1).begin(), cadmium::get_messages<typename input_stream::out>(outmb1).end(), [](const boost::any& m ){ string s = boost::any_cast<string>(m); return s.compare("world")==0;}));
    pf.internal_transition();
    BOOST_CHECK_EQUAL(pf.time_advance(), Time(1));
    //last item
    auto outmb2 = pf.output();
    BOOST_REQUIRE_EQUAL(cadmium::get_messages<typename input_stream::out>(outmb2).size(), 2);
    BOOST_CHECK(any_of(cadmium::get_messages<typename input_stream::out>(outmb2).begin(), cadmium::get_messages<typename input_stream::out>(outmb2).end(), [](const boost::any& m ){ string s = boost::any_cast<string>(m); return s.compare("hello")==0;}));
    BOOST_CHECK(any_of(cadmium::get_messages<typename input_stream::out>(outmb2).begin(), cadmium::get_messages<typename input_stream::out>(outmb2).end(), [](const boost::any& m ){ string s = boost::any_cast<string>(m); return s.compare("world")==0;}));
    pf.internal_transition();
    BOOST_CHECK( isinf(pf.time_advance()));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
