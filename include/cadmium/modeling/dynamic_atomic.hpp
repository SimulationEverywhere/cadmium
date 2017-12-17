#ifndef CADMIUM_DYNAMIC_ATOMIC_HPP
#define CADMIUM_DYNAMIC_ATOMIC_HPP

#include <map>
#include <boost/any.hpp>
#include <cadmium/modeling/atomic.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/concept/concept_helpers.hpp>
#include "dynamic_atomic_helpers.hpp"

namespace cadmium {
    namespace modeling {

        using cadmium::dynamic_bag;

        template<typename TIME, template<typename T> class ATOMIC>
        class dynamic_atomic : atomic {

            // Required types interface
            using state_type = typename ATOMIC<TIME>::state_type;
            using output_ports = typename ATOMIC<TIME>::output_ports;
            using input_ports = typename ATOMIC<TIME>::input_ports;

            using output_bags = typename make_message_bags<output_ports>::type;
            using input_bags = typename make_message_bags<input_ports>::type;

            // wrapped atomic model;
            ATOMIC<TIME> model;

        public:
            // Required members interface
            state_type state;

            dynamic_atomic() {
                static_assert(cadmium::concept::is_atomic<ATOMIC>::value, "This is not an atomic model");
                state = model.state;
            };

            void internal_transition() {
                this->model.internal_transition();
                state = model.state;
            };

            void external_transition(TIME e, std::map<std::type_index, boost::any> bag) {

                input_bags bags;
                cadmium::modeling::fill_bags_from_map(bag, bags);
                this->model.external_transition(e, bags);
                state = model.state;
            };

            void confluence_transition(TIME e, std::map<std::type_index, boost::any> bag) {

                input_bags bags;
                cadmium::modeling::fill_bags_from_map(bag, bags);
                this->model.confluence_transition(e, bags);
                state = model.state;
            };

            std::map<std::type_index, boost::any> output() const {

                std::map<std::type_index, boost::any> bags;
                cadmium::modeling::fill_map_from_bags(this->model.output(), bags);
                return bags;
            };

            TIME time_advance() const {
                return this->model.time_advance();
            };
        };

    }
}

#endif // CADMIUM_DYNAMIC_ATOMIC_HPP