#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <cadmium/basic_model/accumulator.hpp>
#include <cadmium/modeling/dynamic_atomic.hpp>

/**
  * This test is for the dynamic atomic class that wraps an atomic model to make it pointer friendly
  */
template<typename TIME>
using int_accumulator=cadmium::basic_models::accumulator<int, TIME>;

BOOST_AUTO_TEST_SUITE( pdevs_dynamic_atomic_test_suite )

    BOOST_AUTO_TEST_CASE(compile_dynamic_atomic_test) {
        int_accumulator<float> model;
        cadmium::modeling::dynamic_atomic<float, int_accumulator> wrapped_model;
        BOOST_CHECK(typeid(model.state) == typeid(wrapped_model.state));
        BOOST_CHECK(model.state == wrapped_model.state);
    }

BOOST_AUTO_TEST_SUITE_END()