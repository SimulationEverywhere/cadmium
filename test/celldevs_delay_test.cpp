/**
 * Copyright (c) 2020, Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
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
#include <cadmium/celldevs/delay_buffer/delay_buffer_factory.hpp>

using namespace cadmium::celldevs;

BOOST_AUTO_TEST_CASE(inertial) {
    auto buffer = delay_buffer_factory<float, int>::create_delay_buffer("inertial");

    // Check initial state
    BOOST_CHECK_EQUAL(buffer->next_state(), 0);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());

    // Pop buffer (should not have any effect)
    buffer->pop_buffer();
    BOOST_CHECK_EQUAL(buffer->next_state(), 0);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());

    // Add new elements to buffer
    int n = 10;
    for (int i = 1; i <= n; i++) {
        buffer->add_to_buffer(i, i * 4);
        BOOST_CHECK_EQUAL(buffer->next_state(), i);
        BOOST_CHECK_EQUAL(buffer->next_timeout(), i * 4);
    }

    // Pop buffer
    buffer->pop_buffer();
    BOOST_CHECK_EQUAL(buffer->next_state(), n);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());
}

BOOST_AUTO_TEST_CASE(transport) {
    auto buffer = delay_buffer_factory<float, int>::create_delay_buffer("transport");

    // Check initial state
    BOOST_CHECK_EQUAL(buffer->next_state(), 0);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());

    // Pop buffer (should not have any effect)
    buffer->pop_buffer();
    BOOST_CHECK_EQUAL(buffer->next_state(), 0);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());

    // Add new elements to buffer
    int n = 10;
    for (int i = 1; i <= n; i++) {
        buffer->add_to_buffer(i, i * 4);
        BOOST_CHECK_EQUAL(buffer->next_state(), 1);
        BOOST_CHECK_EQUAL(buffer->next_timeout(), 4);
    }

    // Pop (gradually) all elements from buffer
    for (int i = 1; i <= n; i++) {
        // Pop buffer
        BOOST_CHECK_EQUAL(buffer->next_state(), i);
        BOOST_CHECK_EQUAL(buffer->next_timeout(), i * 4);
        buffer->pop_buffer();
    }
    BOOST_CHECK_EQUAL(buffer->next_state(), n);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());

    // Add multiple new elements to buffer
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= i; j++) {
            buffer->add_to_buffer(j, i * 4);
            BOOST_CHECK_EQUAL(buffer->next_state(), 1);
            BOOST_CHECK_EQUAL(buffer->next_timeout(), 4);
        }
    }

    // Pop (gradually) all elements from buffer
    for (int i = 1; i <= n; i++) {
        BOOST_CHECK_EQUAL(buffer->next_state(), i);
        BOOST_CHECK_EQUAL(buffer->next_timeout(), i * 4);
        buffer->pop_buffer();
    }
    BOOST_CHECK_EQUAL(buffer->next_state(), n);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());

    // Add multiple new elements to buffer (in inverse time order
    for (int i = n; i >= 1; i--) {
        for (int j = 1; j <= i; j++) {
            buffer->add_to_buffer(j, i * 4);
            BOOST_CHECK_EQUAL(buffer->next_state(), j);
            BOOST_CHECK_EQUAL(buffer->next_timeout(), i * 4);
        }
    }

    // Pop (gradually) all elements from buffer
    for (int i = 1; i <= n; i++) {
        BOOST_CHECK_EQUAL(buffer->next_state(), i);
        BOOST_CHECK_EQUAL(buffer->next_timeout(), i * 4);
        buffer->pop_buffer();
    }
    BOOST_CHECK_EQUAL(buffer->next_state(), n);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());
}

BOOST_AUTO_TEST_CASE(hybrid) {
    auto buffer = delay_buffer_factory<float, int>::create_delay_buffer("hybrid");

    // Check initial state
    BOOST_CHECK_EQUAL(buffer->next_state(), 0);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());

    // Pop buffer (should not have any effect)
    buffer->pop_buffer();
    BOOST_CHECK_EQUAL(buffer->next_state(), 0);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());

    // Add new elements to buffer
    int n = 10;
    for (int i = 1; i <= n; i++) {
        buffer->add_to_buffer(i, i * 4);
        BOOST_CHECK_EQUAL(buffer->next_state(), 1);
        BOOST_CHECK_EQUAL(buffer->next_timeout(), 4);
    }

    // Pop (gradually) all elements from buffer
    for (int i = 1; i <= n; i++) {
        // Pop buffer
        BOOST_CHECK_EQUAL(buffer->next_state(), i);
        BOOST_CHECK_EQUAL(buffer->next_timeout(), i * 4);
        buffer->pop_buffer();
    }
    BOOST_CHECK_EQUAL(buffer->next_state(), n);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());

    // Add multiple new elements to buffer
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= i; j++) {
            buffer->add_to_buffer(j, i * 4);
            BOOST_CHECK_EQUAL(buffer->next_state(), 1);
            BOOST_CHECK_EQUAL(buffer->next_timeout(), 4);
        }
    }

    // Pop (gradually) all elements from buffer
    for (int i = 1; i <= n; i++) {
        BOOST_CHECK_EQUAL(buffer->next_state(), i);
        BOOST_CHECK_EQUAL(buffer->next_timeout(), i * 4);
        buffer->pop_buffer();
    }
    BOOST_CHECK_EQUAL(buffer->next_state(), n);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());

    // Add multiple new elements to buffer (in inverse time order)
    // Note: as the last element must be at the end of the queue, the queue will always have only one element
    for (int i = n; i >= 1; i--) {
        for (int j = 1; j <= i; j++) {
            buffer->add_to_buffer(j, i * 4);
            BOOST_CHECK_EQUAL(buffer->next_state(), j);
            BOOST_CHECK_EQUAL(buffer->next_timeout(), i * 4);
        }
    }

    // Pop single element from buffer
    BOOST_CHECK_EQUAL(buffer->next_state(), 1);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), 4);
    buffer->pop_buffer();

    BOOST_CHECK_EQUAL(buffer->next_state(), 1);
    BOOST_CHECK_EQUAL(buffer->next_timeout(), std::numeric_limits<float>::infinity());
}
