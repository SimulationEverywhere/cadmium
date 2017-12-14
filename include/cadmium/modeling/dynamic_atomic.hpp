#ifndef CADMIUM_DYNAMIC_ATOMIC_HPP
#define CADMIUM_DYNAMIC_ATOMIC_HPP

#include <map>
#include <boost/any.hpp>
#include <cadmium/modeling/atomic.hpp>

namespace cadmium {
    namespace modeling {

        using cadmium::dynamic_bag;

        template<typename TIME, template<typename T> class ATOMIC>
        class dynamic_atomic : atomic {

            // Required types interface
            using state_type = ATOMIC<TIME>::state_type;

            // wrapped atomic model;
            ATOMIC<TIME> model;

        public:
            // Required members interface
            state_type* state;

            dynamic_atomic() {

                state = &model.state; // Aliasing throw pointer
            };

            void internal() {};
            void external(TIME e, std::map<std::string, dynamic_bag> bag) {};
            void confluence_transition(TIME e, std::map<std::string, dynamic_bag> bag) {};
            std::map<std::string, dynamic_bag> output() const {};
            TIME time_advance() const {};
        }

    }
}

#endif // CADMIUM_DYNAMIC_ATOMIC_HPP