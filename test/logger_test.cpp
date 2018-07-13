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
#include <sstream>
#include<boost/test/unit_test.hpp>
#include <cadmium/logger/logger.hpp>
#include <cadmium/logger/common_loggers.hpp>

namespace {
    std::ostringstream oss;
    std::ostringstream oss2;

    struct oss_test_sink_provider{
        static std::ostream& sink(){
            return oss;
        }
    };

    struct oss_test_second_sink_provider{
        static std::ostream& sink(){
            return oss2;
        }
    };

}


BOOST_AUTO_TEST_SUITE( loggers_test_suite )

BOOST_AUTO_TEST_CASE( log_nothing_test )
{
    oss.str("");

    //logger definition
    cadmium::logger::logger<cadmium::logger::logger_info, cadmium::logger::formatter<float>, oss_test_sink_provider> l;

    //log usage in different source
    l.log<cadmium::logger::logger_debug, cadmium::logger::run_info>("nothing to show");

    BOOST_CHECK(oss.str().empty());
}

BOOST_AUTO_TEST_CASE( simple_logger_logs_test )
{
    oss.str("");

    //logger definition
    cadmium::logger::logger<cadmium::logger::logger_info, cadmium::logger::formatter<float>, oss_test_sink_provider> l;

    //log usage in different source
    l.log<cadmium::logger::logger_info, cadmium::logger::run_info>("something to show");

    BOOST_CHECK_EQUAL(oss.str(), "something to show\n");

}

BOOST_AUTO_TEST_CASE( multiple_loggers_test )
{
    oss.str("");
    oss2.str("");
    //loggers definition
    using log1=cadmium::logger::logger<cadmium::logger::logger_info, cadmium::logger::formatter<float>, oss_test_sink_provider>;
    using log2=cadmium::logger::logger<cadmium::logger::logger_debug, cadmium::logger::formatter<float>, oss_test_second_sink_provider>;

    cadmium::logger::multilogger<log1, log2> l;

    //log usage in different source
    l.log<cadmium::logger::logger_info, cadmium::logger::run_info>("some info");
    l.log<cadmium::logger::logger_debug, cadmium::logger::run_info>("some debug");

    BOOST_CHECK_EQUAL(oss.str(), "some info\n");
    BOOST_CHECK_EQUAL(oss2.str(), "some debug\n");

}


BOOST_AUTO_TEST_SUITE_END()
