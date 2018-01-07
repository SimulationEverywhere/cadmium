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

#ifndef CADMIUM_DYNAMIC_MESSAGE_BAG_HPP
#define CADMIUM_DYNAMIC_MESSAGE_BAG_HPP

#include <boost/any.hpp>
#include <typeindex>
#include <map>
#include <vector>
#include <cadmium/modeling/message_bag.hpp>
#include <iostream>

namespace cadmium {
    namespace dynamic {
        using message_bags = std::map<std::type_index, boost::any>;

        class link_abstract {
        public:
            virtual std::type_index from_type_index() const = 0;
            virtual std::type_index from_port_type_index() const = 0;
            virtual std::type_index to_type_index() const = 0;
            virtual std::type_index to_port_type_index() const = 0;
            virtual void route_messages(const cadmium::dynamic::message_bags& bags_from, cadmium::dynamic::message_bags& bags_to) const = 0;
        };

        //TODO(Lao): add logger, check if the logger can be made transparently for the user,
        //TODO: if not, the message should be made in the simulator instead.
        template<typename PORT_FROM, typename PORT_TO>
        class link : public link_abstract {
        public:
            using from_message_type = typename PORT_FROM::message_type;
            using from_message_bag_type = typename cadmium::message_bag<PORT_FROM>;
            using to_message_type = typename PORT_TO::message_type;
            using to_message_bag_type = typename cadmium::message_bag<PORT_TO>;

            link() {
                static_assert(
                        std::is_same<from_message_type, to_message_type>::value,
                        "FROM_PORT message type and TO_PORT message types must be the same type");
            }

            std::type_index from_type_index() const override {
                return typeid(from_message_bag_type);
            }

            std::type_index from_port_type_index() const override {
                return typeid(PORT_FROM);
            }

            std::type_index to_type_index() const override {
                return typeid(to_message_bag_type);
            }

            std::type_index to_port_type_index() const override {
                return typeid(PORT_TO);
            }

            void pass_messages(const boost::any& bag_from, boost::any& bag_to) const {
                from_message_bag_type b_from = boost::any_cast<from_message_bag_type>(bag_from);
                to_message_bag_type* b_to = boost::any_cast<to_message_bag_type>(&bag_to);
                b_to->messages.insert(b_to->messages.end(), b_from.messages.begin(), b_from.messages.end());
            }

            boost::any pass_messages_to_new_bag(const boost::any& bag_from) const {
                from_message_bag_type b_from = boost::any_cast<from_message_bag_type>(bag_from);
                to_message_bag_type b_to;
                b_to.messages.insert(b_to.messages.end(), b_from.messages.begin(), b_from.messages.end());
                return b_to;
            }

            /**
             * @note This methods assumes the port is defined in the message_bagas parameter bag,
             * if is not the case, the function throws a std::map out of range exception.
             *
             * @param bags - The cadmium::dynamic::message_bags to check if there is messages in the from port
             * @return true if there is messages, otherwise false
             */
            bool there_is_messages_to_route(const cadmium::dynamic::message_bags& bags) const {
                return boost::any_cast<from_message_bag_type>(bags.at(this->from_port_type_index())).messages.size() > 0;
            }

            void route_messages(const cadmium::dynamic::message_bags& bags_from, cadmium::dynamic::message_bags& bags_to) const override {
                if (bags_from.find(this->from_port_type_index()) != bags_from.end()) {
                    if (bags_to.find(this->to_port_type_index()) != bags_to.end()) {
                        this->pass_messages(bags_from.at(this->from_port_type_index()), bags_to.at(this->to_port_type_index()));
                    } else if (this->there_is_messages_to_route(bags_from)) {
                        bags_to[this->to_port_type_index()] = this->pass_messages_to_new_bag(bags_from.at(this->from_port_type_index()));
                    }
                }
            }
        };

        template <typename PORT_FROM, typename PORT_TO>
        std::shared_ptr<link_abstract> make_link() {
            std::shared_ptr<link_abstract> spLink = std::make_shared<link<PORT_FROM, PORT_TO>>();
            return spLink;
        }
    }
}

#endif //CADMIUM_DYNAMIC_MESSAGE_BAG_HPP