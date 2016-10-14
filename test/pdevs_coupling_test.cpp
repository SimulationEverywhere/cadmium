/**
 * Copyright (c) 2013-2016, Damian Vicino
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
#include <cadmium/concept/atomic_model_assert.hpp>
#include <cadmium/modeling/coupled_model.hpp>
#include <cadmium/basic_model/accumulator.hpp>


//using empty_tuple=std::tuple<>;

//template<typename TIME>
//using int_accumulator=cadmium::basic_models::accumulator<int, TIME>;
//
//
//using empty_models=cadmium::modeling::models_tuple<>;
//
//template<typename T>
//using C1=cadmium::modeling::coupled_model<T, empty_tuple, empty_tuple, empty_models, empty_tuple, empty_tuple, empty_tuple>;


//this test suite has a concept check per each concept check defined using the archetypes
BOOST_AUTO_TEST_SUITE( pdevs_coupling_suite )

BOOST_AUTO_TEST_CASE( pdevs_single_model_no_ports_test )
{
    BOOST_FAIL("Unimplemented");
}

BOOST_AUTO_TEST_CASE( pdevs_generators_to_outputs)
{
    BOOST_FAIL("Unimplemented");
}

BOOST_AUTO_TEST_CASE( pdevs_inputs_to_null )
{
    BOOST_FAIL("Unimplemented");
}


BOOST_AUTO_TEST_CASE( pdevs_inputs_to_null_and_generators_to_outputs )
{
    BOOST_FAIL("Unimplemented");
}

BOOST_AUTO_TEST_CASE( pdevs_inputs_to_accumulator_generator_to_acumulator_acumulator_to_output )
{
    BOOST_FAIL("Unimplemented");
}


BOOST_AUTO_TEST_SUITE_END()

