#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <cadmium/basic_model/accumulator.hpp>
#include <cadmium/engine/pdevs_simulator.hpp>
#include <cadmium/engine/pdevs_coordinator.hpp>
#include <cadmium/basic_model/generator.hpp>
#include <cadmium/modeling/dynamic_atomic_helpers.hpp>
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
            bs_map[typeid(bag_0)] = bag_0;
            bs_map[typeid(bag_1)] = bag_1;

            using test_input_ports=std::tuple<test_in_0, test_in_1>;
            using input_bags=typename cadmium::make_message_bags<test_input_ports>::type;

            input_bags bs_tuple;
            cadmium::modeling::fill_bags_from_map<input_bags>(bs_map, bs_tuple);
            BOOST_CHECK_EQUAL(bag_0.messages.size(), cadmium::get_messages<test_in_0>(bs_tuple).size());
            BOOST_CHECK_EQUAL(bag_1.messages.size(), cadmium::get_messages<test_in_>(bs_tuple).size());
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

            cadmium::modeling::fill_map_from_bags<input_bags>(bs_tuple, bs_map);

            bag_0 tuple_bag_0 = std::get<0>(bs_tuple);
            bag_0 map_bag_0 = boost::any_cast<bag_0>(bs_map[typeid(tuple_bag_0)]);
            BOOST_CHECK_EQUAL(map_bag_0.messages.size(), tuple_bag_0.messages.size());

            bag_1 tuple_bag_1 = std::get<1>(bs_tuple);
            bag_1 map_bag_1 = boost::any_cast<bag_1>(bs_map[typeid(tuple_bag_1)]);
            BOOST_CHECK_EQUAL(map_bag_1.messages.size(), tuple_bag_1.messages.size());
    }


BOOST_AUTO_TEST_SUITE_END()
