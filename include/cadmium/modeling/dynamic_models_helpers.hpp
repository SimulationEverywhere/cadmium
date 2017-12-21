/**
 * Copyright (c) 2017, Laouen M. L. Belloli
 * Carleton University, Universidad de Buenos Aires
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

#ifndef CADMIUM_DYNAMIC_ATOMIC_HELPERS_HPP
#define CADMIUM_DYNAMIC_ATOMIC_HELPERS_HPP

#include <tuple>
#include <typeindex>
#include <boost/any.hpp>
#include <map>
#include <memory>

namespace cadmium {
    namespace modeling {

        using models_map=std::map<std::type_index, std::shared_ptr<model>>;
        using ports_vector=std::vector<std::type_index>;
        using links_vector=std::vector<std::vector<std::type_index>>;

        // Generic tuple for_each function
        template<typename TUPLE, typename FUNC>
        void for_each(TUPLE& ts, FUNC&& f) {

            auto for_each_fold_expression = [&f](auto &... e)->void { (f(e) , ...); };
            std::apply(for_each_fold_expression, ts);
        }

        /**
         * @brief Insert all the previously casted message bags of bags in the typed bs message bags.
         *
         * @tparam BST The message bag tuple to fill from the dynamic_message_bags.
         * @param bags - The dynamic_message_bags that carries the message to be placed in the bs parameter.
         * @param bs  - The BST message bags that will be filled with the bags messages.
         */
        template<typename BST>
        void fill_bags_from_map(cadmium::dynamic_message_bags& bags, BST& bs) {

            auto add_messages_to_bag = [&bags](auto & b)->void {
                using bag_type = decltype(b);

                bag_type b2 = boost::any_cast<bag_type>(bags.at(typeid(bag_type)));
                b.messages.insert(b.messages.end(), b2.messages.begin(), b2.messages.end());
            };
            for_each<BST>(bs, add_messages_to_bag);
        }

        /**
         * @brief Insert all the message bs of bags in bags by an implicit conversion of them to boost::any.
         *
         * @tparam BST The message bag tuple that carries the messages to fill the dynamic_message_bags.
         * @param bags - The dynamic_message_bag that will be filled with the bs messages.
         * @param bs  - The BST message bag that carries the message to be placed in the bags parameter.
         */
        template<typename BST>
        void fill_map_from_bags(BST& bs, cadmium::dynamic_message_bags& bags) {

            auto add_messages_to_map = [&bags](auto & b)->void {
                using bag_type = decltype(b);

                bags[typeid(bag_type)] = b;
            };
            for_each<BST>(bs, add_messages_to_map);
        }

        bool is_in(const std::type_index& port, const ports_vector& ports) {
            return std::find(ports.cbegin(), ports.cend(), port) != ports.cend();
        }

        bool valid_ic_links(const models_map& models, const links_vector& ic) {
            for (auto & link : ic) {
                if (link.size() != 4 || models.find(link[0]) == models.cend() || models.find(link[3]) == models.cend()) {
                    return false;
                }
            }
            return true;
        }

        bool valid_eic_links(const models_map& models, const ports_vector& input_ports, const links_vector& eic) {
            for (auto & link : eic) {
                if (link.size() != 3 || models.find(link[1]) == models.cend() || is_in(link[0], input_ports)) {
                    return false;
                }
            }
            return true;
        }

        bool valid_eoc_links(const models_map& models, const ports_vector& output_ports, const links_vector& eoc) {
            for (auto & link : eoc) {
                if (link.size() != 3 || models.find(link[0]) == models.cend() || is_in(link[2], output_ports)) {
                    return false;
                }
            }
            return true;
        }
    }
}

#endif //CADMIUM_DYNAMIC_ATOMIC_HELPERS_HPP
