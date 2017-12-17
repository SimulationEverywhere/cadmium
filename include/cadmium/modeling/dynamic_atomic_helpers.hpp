#ifndef CADMIUM_DYNAMIC_ATOMIC_HELPERS_HPP
#define CADMIUM_DYNAMIC_ATOMIC_HELPERS_HPP

#include <tuple>
#include <typeindex>
#include <boost/any.hpp>
#include <map>

namespace cadmium {
    namespace modeling {

        //Generic tuple for_each function
        template<typename TUPLE, typename FUNC>
        void for_each(TUPLE& ts, FUNC&& f) {

            auto for_each_fold_expression = [&f](auto &... e)->void { (f(e) , ...); };
            std::apply(for_each_fold_expression, ts);
        }

        template<typename BST>
        void fill_bags_from_map(std::map<std::type_index, boost::any>& bags, BST& bs) {

            auto add_messages_to_bag = [&bags](auto & b)->void {
                using bag_type = decltype(b);

                bag_type b2 = boost::any_cast<bag_type>(bags.at(typeid(bag_type)));
                b.messages.insert(b.messages.end(), b2.messages.begin(), b2.messages.end());
            };
            for_each<BST>(bs, add_messages_to_bag);
        }

        template<typename BST>
        void fill_map_from_bags(BST& bs, std::map<std::type_index, boost::any>& bags) {

            auto add_messages_to_map = [&bags](auto & b)->void {
                using bag_type = decltype(b);

                bags[typeid(bag_type)] = b;
            };
            for_each<BST>(bs, add_messages_to_map);
        }
    }
}

#endif //CADMIUM_DYNAMIC_ATOMIC_HELPERS_HPP
