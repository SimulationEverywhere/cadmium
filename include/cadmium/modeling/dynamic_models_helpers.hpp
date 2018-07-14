/**
 * Copyright (c) 2018, Laouen M. L. Belloli
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
#include <algorithm>

#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/engine/common_helpers.hpp>

namespace cadmium {
    namespace dynamic {
        namespace modeling {

            /**
             * @brief Takes a dynamic message bag and using the port tuple type it casts all the
             * message bas type to their original type and prints the messages in the ostream object
             * using the cadmium implode logger helper.
             *
             * @note normally this function would be a logger helper, but because the dynamic atomic
             * model encapsulates the model type and this uses the model type, then, the dynamic
             * atomic model is the responsible for providing this method. It is not a good desing, but
             * it is as far as we know, the only way.
             *
             * @tparam BST - The std::tuple<PORT_TYPE..> type
             * @param os - the ostream used as sink where to print the messages by port.
             * @param bags - the cadmium::dynamic::bag with the messages
             */
            template<typename BST>
            void print_dynamic_messages_by_port(std::ostream& os, cadmium::dynamic::message_bags& bags) {

                int a = 0; // to dynamically count tuple index
                auto print_messages_fold_expr = [&a, &bags, &os](auto b) -> void {
                    using port_type = decltype(b);
                    using bag_type = typename cadmium::message_bag<port_type>;

                    if (bags.find(typeid(port_type)) == bags.cend()) {
                        // there is no messages in the port
                        return;
                    }

                    if (a != 0) {
                        os << ", ";
                    }

                    bag_type& casted_bag = boost::any_cast<bag_type&>(bags.at(typeid(port_type)));
                    os << boost::typeindex::type_id<port_type>().pretty_name();
                    os << ": ";
                    cadmium::logger::implode(os, casted_bag.messages);

                    a++;
                };
                os << "[";
                BST bs;
                cadmium::helper::for_each<BST>(bs, print_messages_fold_expr);
                os << "]";
            }


            /**
             * @brief Constructs an empty dynamic_message_bag with all the bs tuple members as keys of empties message bags.
             *
             * @tparam BST The message bag tuple.
             */
            template<typename BST>
            cadmium::dynamic::message_bags create_empty_message_bags() {

                cadmium::dynamic::message_bags bags;
                auto create_empty_bag = [&bags](auto b) -> void {
                    using bag_type = decltype(b);
                    using port_type = typename bag_type::port;

                    bags[typeid(port_type)] = b;
                };
                BST bs;
                cadmium::helper::for_each<BST>(bs, create_empty_bag);
                return bags;
            }

            /**
             * @brief Constructs a cadmium::dynamic::Ports all the bs tuple members type_idex.
             *
             * @tparam BST The message bag tuple.
             */
            template<typename BST>
            cadmium::dynamic::modeling::Ports create_dynamic_ports() {

                cadmium::dynamic::modeling::Ports ret;
                auto create_empty_bag = [&ret](auto b) -> void {
                    std::type_index port_type_index = typeid(decltype(b));
                    ret.push_back(port_type_index);
                };
                BST bs;
                cadmium::helper::for_each<BST>(bs, create_empty_bag);
                return ret;
            }

            /**
             * @brief Insert all the previously casted message bags of bags in the typed bs message bags.
             *
             * @tparam BST The message bag tuple to fill from the cadmium::dynamic::message_bags.
             * @param bags - The cadmium::dynamic::message_bags that carries the message to be placed in the bs parameter.
             * @param bs  - The BST message bags that will be filled with the bags messages.
             */
            template<typename BST>
            void fill_bags_from_map(cadmium::dynamic::message_bags &bags, BST &bs) {

                auto add_messages_to_bag = [&bags, &bs](auto b) -> void {
                    using bag_type = decltype(b);
                    using port_type = typename bag_type::port;

                    if (bags.find(typeid(port_type)) != bags.end()) {
                        bag_type b2 = boost::any_cast<bag_type>(bags.at(typeid(port_type)));
                        auto& current_bag = cadmium::get_messages<port_type>(bs);
                        current_bag.insert(
                                current_bag.end(),
                                b2.messages.begin(),
                                b2.messages.end()
                        );
                    }
                };
                cadmium::helper::for_each<BST>(bs, add_messages_to_bag);
            }

            /**
             * @brief Insert all the message bs of bags in bags by an implicit conversion of them to boost::any.
             *
             * @tparam BST The message bag tuple that carries the messages to fill the cadmium::dynamic::message_bags.
             * @param bags - The dynamic_message_bag that will be filled with the bs messages.
             * @param bs  - The BST message bag that carries the message to be placed in the bags parameter.
             */
            template<typename BST>
            void fill_map_from_bags(BST &bs, cadmium::dynamic::message_bags &bags) {

                auto add_messages_to_map = [&bags](auto b) -> void {
                    using bag_type = decltype(b);
                    using port_type = typename bag_type::port;

                    bags[typeid(port_type)] = b;
                };
                cadmium::helper::for_each<BST>(bs, add_messages_to_map);
            }

            bool is_in(const std::type_index &port, const Ports &ports) {
                return std::find(ports.cbegin(), ports.cend(), port) != ports.cend();
            }

            bool valid_ic_links(const Models &models, const ICs &ic) {
                return std::all_of(ic.cbegin(), ic.cend(), [&models](const auto &link) -> bool {
                    return std::find_if(models.cbegin(), models.cend(),
                                        [&link](const auto &m) -> bool {
                                            cadmium::dynamic::modeling::Ports output_ports = m->get_output_ports();
                                            std::type_index from_port_type = link._link->from_port_type_index();
                                            return m->get_id() == link._from &&
                                                    std::find(
                                                            output_ports.begin(),
                                                            output_ports.end(),
                                                            from_port_type
                                                    ) != output_ports.end();
                                        }) != models.cend() &&
                           std::find_if(models.cbegin(), models.cend(),
                                        [&link](const auto &m) -> bool {
                                            cadmium::dynamic::modeling::Ports input_ports = m->get_input_ports();
                                            std::type_index to_port_type = link._link->to_port_type_index();
                                            return m->get_id() == link._to &&
                                                   std::find(
                                                           input_ports.begin(),
                                                           input_ports.end(),
                                                           to_port_type
                                                   ) != input_ports.end();
                                        }) != models.cend();
                });
            }

            bool valid_eic_links(const Models &models, const Ports &input_ports, const EICs &eic) {
                return std::all_of(eic.cbegin(), eic.cend(),
                                   [&models, &input_ports](const auto &link) -> bool {
                                       return std::find_if(
                                               models.cbegin(), models.cend(),
                                               [&link](const auto &m) -> bool {
                                                   cadmium::dynamic::modeling::Ports input_ports = m->get_input_ports();
                                                   std::type_index to_port_type = link._link->to_port_type_index();
                                                   return m->get_id() == link._to &&
                                                           std::find(
                                                                   input_ports.begin(),
                                                                   input_ports.end(),
                                                                   to_port_type
                                                           ) != input_ports.end();
                                               }) != models.cend() &&
                                              is_in(link._link->from_port_type_index(), input_ports);
                                   });
            }

            bool valid_eoc_links(const Models &models, const Ports &output_ports, const EOCs &eoc) {
                return std::all_of(eoc.cbegin(), eoc.cend(),
                                   [&models, &output_ports](const auto &link) -> bool {
                                       return std::find_if(
                                               models.cbegin(),
                                               models.cend(),
                                               [&link](const auto &m) -> bool { // searches for a model with the same id and port
                                                   cadmium::dynamic::modeling::Ports output_ports = m->get_output_ports();
                                                   std::type_index from_port_type = link._link->from_port_type_index();
                                                   return m->get_id() == link._from &&
                                                           std::find(
                                                                   output_ports.begin(),
                                                                   output_ports.end(),
                                                                   from_port_type
                                                           ) != output_ports.end();
                                               }) != models.cend() &&
                                              is_in(link._link->to_port_type_index(), output_ports);
                                   });
            }
        }
    }
}

#endif //CADMIUM_DYNAMIC_ATOMIC_HELPERS_HPP
