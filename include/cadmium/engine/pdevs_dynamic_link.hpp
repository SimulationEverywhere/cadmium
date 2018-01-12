//
// Created by lao on 12/01/18.
//

#ifndef CADMIUM_PDEVS_DYNAMIC_LINK_HPP
#define CADMIUM_PDEVS_DYNAMIC_LINK_HPP

#include <typeindex>
#include <memory>

#include <cadmium/logger/dynamic_common_loggers.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/common_loggers_helpers.hpp>

namespace cadmium {
    namespace dynamic {
        namespace engine {

            class link_abstract {
            public:
                virtual std::type_index from_type_index() const = 0;

                virtual std::type_index from_port_type_index() const = 0;

                virtual std::type_index to_type_index() const = 0;

                virtual std::type_index to_port_type_index() const = 0;

                virtual cadmium::dynamic::logger::routed_messages
                route_messages(const cadmium::dynamic::message_bags &bags_from,
                               cadmium::dynamic::message_bags &bags_to) const = 0;
            };

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

                cadmium::dynamic::logger::routed_messages
                pass_messages(const boost::any &bag_from, boost::any &bag_to) const {
                    from_message_bag_type b_from = boost::any_cast<from_message_bag_type>(bag_from);
                    to_message_bag_type *b_to = boost::any_cast<to_message_bag_type>(&bag_to);
                    b_to->messages.insert(b_to->messages.end(), b_from.messages.begin(),
                                          b_from.messages.end());

                    return cadmium::dynamic::logger::routed_messages(
                            cadmium::logger::messages_as_strings(b_from.messages),
                            cadmium::logger::messages_as_strings(b_to->messages),
                            boost::typeindex::type_id<PORT_FROM>().pretty_name(),
                            boost::typeindex::type_id<PORT_TO>().pretty_name()
                    );
                }

                cadmium::dynamic::logger::routed_messages
                pass_messages_to_new_bag(const boost::any &bag_from,
                                         cadmium::dynamic::message_bags &bags_to) const {
                    from_message_bag_type b_from = boost::any_cast<from_message_bag_type>(bag_from);
                    to_message_bag_type b_to;
                    b_to.messages.insert(b_to.messages.end(), b_from.messages.begin(),
                                         b_from.messages.end());
                    bags_to[this->to_port_type_index()] = b_to;

                    return cadmium::dynamic::logger::routed_messages(
                            cadmium::logger::messages_as_strings(b_from.messages),
                            cadmium::logger::messages_as_strings(b_to.messages),
                            boost::typeindex::type_id<PORT_FROM>().pretty_name(),
                            boost::typeindex::type_id<PORT_TO>().pretty_name()
                    );
                }

                /**
                 * @note This methods assumes the port is defined in the message_bags parameter bag,
                 * if is not the case, the function throws a std::map out of range exception.
                 *
                 * @param bags - The cadmium::dynamic::message_bags to check if there is messages in the from port
                 * @return true if there is messages, otherwise false
                 */
                bool is_there_messages_to_route(const cadmium::dynamic::message_bags &bags) const {
                    return boost::any_cast<from_message_bag_type>(
                            bags.at(this->from_port_type_index())).messages.size() > 0;
                }

                cadmium::dynamic::logger::routed_messages
                route_messages(const cadmium::dynamic::message_bags &bags_from,
                               cadmium::dynamic::message_bags &bags_to) const override {
                    if (bags_from.find(this->from_port_type_index()) != bags_from.cend()) {

                        if (bags_to.find(this->to_port_type_index()) != bags_to.cend()) {
                            return this->pass_messages(bags_from.at(this->from_port_type_index()),
                                                       bags_to.at(this->to_port_type_index()));
                        }

                        if (this->is_there_messages_to_route(bags_from)) {
                            return this->pass_messages_to_new_bag(
                                    bags_from.at(this->from_port_type_index()), bags_to);
                        }

                        cadmium::dynamic::logger::routed_messages empty_ret;
                        return empty_ret; // if no messages where routed, it returns an empty vector
                    }
                }
            };

            template<typename PORT_FROM, typename PORT_TO>
            std::shared_ptr<link_abstract> make_link() {
                std::shared_ptr<link_abstract> spLink = std::make_shared<link<PORT_FROM, PORT_TO>>();
                return spLink;
            }
        }
    }
}


#endif //CADMIUM_PDEVS_DYNAMIC_LINK_HPP
