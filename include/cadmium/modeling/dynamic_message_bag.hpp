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

namespace cadmium {
    namespace dynamic {
        using message_bags = std::map<std::type_index, boost::any>;

        class link_abstract {
        public:
            virtual std::type_index from_type_index() const = 0;
            virtual std::type_index to_type_index() const = 0;
            virtual void pass_messages(boost::any& bag_from, boost::any& bag_to) const = 0;
            virtual boost::any pass_messages_to_new_bag(boost::any& bag_from) const = 0;
        };

        template<typename PORT_FROM, typename PORT_TO>
        class link : public link_abstract {
        public:
            using from_message_type = typename PORT_FROM::message_type;
            using from_message_bag_type = typename cadmium::message_bag<PORT_FROM>;
            using to_message_type = typename PORT_TO::message_type;
            using to_message_bag_type = typename cadmium::message_bag<PORT_TO>;

            link() {}

            // TODO(Lao): check if the any_cast with the assignment keeps the reference or create a copy
            void pass_messages(boost::any& bag_from, boost::any& bag_to) const {
                from_message_bag_type b_from = boost::any_cast<from_message_bag_type>(bag_from);
                to_message_bag_type* b_to = boost::any_cast<to_message_bag_type>(&bag_to);
                b_to->messages.insert(b_to->messages.end(), b_from.messages.begin(), b_from.messages.end());
            }

            boost::any pass_messages_to_new_bag(boost::any& bag_from) const {
                from_message_bag_type b_from = boost::any_cast<from_message_bag_type>(bag_from);
                to_message_bag_type b_to;
                b_to.messages.insert(b_to.messages.end(), b_from.messages.begin(), b_from.messages.end());
                return b_to;
            }

            std::type_index from_type_index() const {
                return typeid(from_message_bag_type);
            }

            std::type_index to_type_index() const {
                return typeid(to_message_bag_type);
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