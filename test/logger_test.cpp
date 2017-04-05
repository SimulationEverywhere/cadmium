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

    struct oss_test_sink_provider{
        static std::ostream& sink(){
            return oss;
        }
    };
}


BOOST_AUTO_TEST_SUITE( loggers_test_suite )

BOOST_AUTO_TEST_CASE( log_nothing_test )
{
    oss.clear();

    //logger definition
    cadmium::logger::logger<cadmium::logger::logger_info, cadmium::logger::verbatim_formatter, oss_test_sink_provider> l;

    //log usage in different source
    l.log<cadmium::logger::logger_debug, std::string>("nothing to show");

    BOOST_CHECK(oss.str().empty());
}

BOOST_AUTO_TEST_CASE( simple_logger_logs_test )
{
    oss.clear();

    //logger definition
    cadmium::logger::logger<cadmium::logger::logger_info, cadmium::logger::verbatim_formatter, oss_test_sink_provider> l;

    //log usage in different source
    l.log<cadmium::logger::logger_info, std::string>("nothing to show");

    BOOST_CHECK_EQUAL(oss.str(), "nothing to show\n");

}

BOOST_AUTO_TEST_CASE( multiple_loggers_test )
{
    BOOST_WARN("Not yet implemented, this test is a placeholder");
}


BOOST_AUTO_TEST_SUITE_END()
