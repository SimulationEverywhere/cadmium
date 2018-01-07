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
#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/modeling/model.hpp>

namespace cadmium {
    namespace dynamic {
        namespace modeling {

            // Generic tuple for_each function
            template<typename TUPLE, typename FUNC>
            void for_each(TUPLE &ts, FUNC &&f) {

                auto for_each_fold_expression = [&f](auto &... e) -> void { (f(e), ...); };
                std::apply(for_each_fold_expression, ts);
            }

            struct EOC {
                std::string _from;
                std::shared_ptr<cadmium::dynamic::link_abstract> _link;

                EOC() = delete;

                EOC(std::string from, std::shared_ptr<cadmium::dynamic::link_abstract> l)
                        : _from(from), _link(l) {}

                EOC(const EOC& other)
                        : _from(other._from), _link(other._link) {}
            };

            struct EIC {
                std::string _to;
                std::shared_ptr<cadmium::dynamic::link_abstract> _link;

                EIC() = delete;

                EIC(std::string to, std::shared_ptr<cadmium::dynamic::link_abstract> l)
                        : _to(to), _link(l) {}

                EIC(const EIC& other)
                        : _to(other._to), _link(other._link) {}
            };

            struct IC {
                std::string _to;
                std::string _from;
                std::shared_ptr<cadmium::dynamic::link_abstract> _link;

                IC() = delete;

                IC(std::string to, std::string from, std::shared_ptr<cadmium::dynamic::link_abstract> l)
                        : _to(to), _from(from), _link(l) {}

                IC(const IC& other)
                        : _to(other._to), _from(other._from), _link(other._link) {}
            };

            using dynamic_EC=std::tuple<std::type_index, std::type_index, std::type_index>;
            using dynamic_IC=std::tuple<std::type_index, std::type_index, std::type_index, std::type_index>;

            using Models = std::vector<std::shared_ptr<cadmium::dynamic::modeling::model>>;
            using Ports = std::vector<std::type_index>;
            using EICs = std::vector<EIC>;
            using EOCs = std::vector<EOC>;
            using ICs = std::vector<IC>;

            using initializer_list_Models = std::initializer_list<std::shared_ptr<cadmium::dynamic::modeling::model>>;
            using initilizer_list_Ports = std::initializer_list<std::type_index>;
            using initializer_list_EOCs = std::initializer_list<EOC>;
            using initializer_list_EICs = std::initializer_list<EIC>;
            using initializer_list_ICs = std::initializer_list<IC>;

            template<typename PORTS>
            cadmium::dynamic::modeling::Ports make_ports() {
                cadmium::dynamic::modeling::Ports ret;
                auto add_port = [&ret] (auto &p) -> void {
                    std::type_index port_type_index = typeid(p);
                    ret.push_back(port_type_index);
                };
                PORTS ports;
                for_each<PORTS>(ports, add_port);
                return ret;
            }

            /**
             * @brief Constructs the correct cadmium::dynamic::modeling::EICs object from the original cadmium std::tuple<EIC..> type
             *
             * @pre The original cadmium EIC types must contain a model with default constructor and with the method get_id()
             * for this purpose coupled models can be made as a derived class from cadmium::dynamic::modeling::coupled with the constructor
             * set with a custom id.
             *
             * @tparam EICS - the type of the original cadmium EIC tuple.
             * @tparam TIME - the model time type
             * @return The constructed cadmium::dynamic::modeling::EICs object
             */
            template<typename EICS, typename TIME>
            cadmium::dynamic::modeling::EICs make_EICs() {
                cadmium::dynamic::modeling::EICs ret;
                auto add_eic = [&ret] (auto &e) -> void {

                    using EIC_type = decltype(e);
                    using model_type = typename EIC_type::template submodel<TIME>;
                    using from_port_type = typename EIC_type::external_input_port;
                    using to_port_type = typename EIC_type::submodel_input_port;

                    //Note: models must have a default constructor if is not the case, it will not work.
                    model_type sp_model = std::make_shared<model_type>();
                    std::string to = sp_model->get_id();

                    std::shared_ptr<cadmium::dynamic::link_abstract> new_link = cadmium::dynamic::make_link<from_port_type, to_port_type>();
                    ret.emplace_back(to, new_link);
                };
                EICS eics;
                for_each<EICS>(eics, add_eic);
                return ret;
            }

            /**
             * @brief Constructs the correct cadmium::dynamic::modeling::EOCs object from the original cadmium std::tuple<EOC..> type
             *
             * @pre The original cadmium EOC types must contain a model with default constructor and with the method get_id()
             * for this purpose coupled models can be made as a derived class from cadmium::dynamic::modeling::coupled with the constructor
             * set with a custom id.
             *
             * @tparam EOCS - the type of the original cadmium EIC tuple.
             * @tparam TIME - the model time type
             * @return The constructed cadmium::dynamic::modeling::EICs object
             */
            template<typename EOCS, typename TIME>
            cadmium::dynamic::modeling::EOCs make_EOCs() {
                cadmium::dynamic::modeling::EOCs ret;
                auto add_eoc = [&ret] (auto &e) -> void {

                    using EOC_type = decltype(e);
                    using model_type = typename EOC_type::template submodel<TIME>;
                    using from_port_type = typename EOC_type::submodel_output_port;
                    using to_port_type = typename EOC_type::external_output_port;

                    //Note: models must have a default constructor if is not the case, it will not work.
                    model_type sp_model = std::make_shared<model_type>();
                    std::string to = sp_model->get_id();

                    std::shared_ptr<cadmium::dynamic::link_abstract> new_link = cadmium::dynamic::make_link<from_port_type, to_port_type>();
                    ret.emplace_back(to, new_link);
                };
                EOCS eocs;
                for_each<EOCS>(eocs, add_eoc);
                return ret;
            }

            /**
             * @brief Constructs an empty dynamic_message_bag with all the bs tuple members as keys of empties message bags.
             *
             * @tparam BST The message bag tuple.
             */
            template<typename BST>
            cadmium::dynamic::message_bags create_empty_message_bags() {

                cadmium::dynamic::message_bags bags;
                auto create_empty_bag = [&bags](auto &b) -> void {
                    using bag_type = decltype(b);

                    bags[typeid(bag_type)] = b;
                };
                BST bs;
                for_each<BST>(bs, create_empty_bag);
                return bags;
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

                auto add_messages_to_bag = [&bags](auto &b) -> void {
                    using bag_type = decltype(b);

                    if (bags.find(typeid(b)) != bags.end()) {
                        bag_type b2 = boost::any_cast<bag_type>(bags.at(typeid(b)));
                        b.messages.insert(b.messages.end(), b2.messages.begin(), b2.messages.end());
                    }
                };
                for_each<BST>(bs, add_messages_to_bag);
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

                auto add_messages_to_map = [&bags](auto &b) -> void {
                    using bag_type = decltype(b);

                    bags[typeid(bag_type)] = b;
                };
                for_each<BST>(bs, add_messages_to_map);
            }

            bool is_in(const std::type_index &port, const Ports &ports) {
                return std::find(ports.cbegin(), ports.cend(), port) != ports.cend();
            }

            bool valid_ic_links(const Models &models, const ICs &ic) {
                return std::all_of(ic.cbegin(), ic.cend(), [&models](const auto &link) -> bool {
                    return std::find_if(models.cbegin(), models.cend(),
                                        [&link](const auto &m) -> bool {
                                            return m->get_id() == link._from;
                                        }) != models.cend() &&
                           std::find_if(models.cbegin(), models.cend(),
                                        [&link](const auto &m) -> bool {
                                            return m->get_id() == link._to;
                                        }) != models.cend();
                });
            }

            bool valid_eic_links(const Models &models, const Ports &input_ports, const EICs &eic) {
                return std::all_of(eic.cbegin(), eic.cend(),
                                   [&models, &input_ports](const auto &link) -> bool {
                                       return std::find_if(models.cbegin(), models.cend(),
                                                           [&link](const auto &m) -> bool {
                                                               return m->get_id() == link._to;
                                                           }) != models.cend() &&
                                              is_in(link._link->from_type_index(), input_ports);
                                   });
            }

            bool valid_eoc_links(const Models &models, const Ports &output_ports, const EOCs &eoc) {
                return std::all_of(eoc.cbegin(), eoc.cend(),
                                   [&models, &output_ports](const auto &link) -> bool {
                                       return std::find_if(models.cbegin(), models.cend(),
                                                           [&link](const auto &m) -> bool {
                                                               return m->get_id() == link._from;
                                                           }) != models.cend() &&
                                              is_in(link._link->to_type_index(), output_ports);
                                   });
            }
        }
    }
}

#endif //CADMIUM_DYNAMIC_ATOMIC_HELPERS_HPP
