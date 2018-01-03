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

#ifndef CADMIUM_PDEVS_DYNAMIC_ENGINE_HELPERS_HPP
#define CADMIUM_PDEVS_DYNAMIC_ENGINE_HELPERS_HPP

#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <boost/any.hpp>
#include <cadmium/engine/pdevs_dynamic_engine.hpp>

namespace cadmium {
    namespace dynamic {
        namespace engine {
            //Checking all dynamic bag of inbox or outbox are empty
            template<class BOX>
            decltype(auto) all_bags_empty(cadmium::dynamic::message_bags const& dynamic_bag) {
                auto is_empty = [&dynamic_bag](auto const& b) -> bool {
                    using bag_type = decltype(b);
                    if (dynamic_bag.find(typeid(b)) != dynamic_bag.cend()) {
                        return boost::any_cast<bag_type>(dynamic_bag.at(typeid(b))).messages.empty();
                    }
                    // A not declared bag in the dynamic_bag is the same as a bag with empty messages
                    return true;
                };
                auto check_empty = [&is_empty](auto const&... b)->decltype(auto) {
                    return (is_empty(b) && ...);
                };

                // the box is used to get all the port types to use as keys.
                BOX box;
                return std::apply(check_empty, box);
            }

            template<typename TIME>
            using subcoordinators_type = typename std::vector<std::shared_ptr<cadmium::dynamic::engine::engine<TIME>>>;

            template<typename TIME>
            void init_subcoordinators(TIME t, subcoordinators_type<TIME>& subcoordinators) {
                auto init_coordinator = [&t](auto & c)->void { c->init(t); };
                std::for_each(subcoordinators.begin(), subcoordinators.end(), init_coordinator);
            }

            template<typename TIME>
            void collect_outputs_in_subcoordinators(TIME t, subcoordinators_type<TIME>& subcoordinators) {
                auto collect_output = [&t](auto & c)->void { c->collect_outputs(t); };
                std::for_each(subcoordinators.begin(), subcoordinators.end(), collect_output);
            }

            template<typename TIME>
            TIME min_next_in_subcoordinators(const subcoordinators_type<TIME>& subcoordinators) {
                std::vector<TIME> next_times(subcoordinators.size());
                std::transform(
                        subcoordinators.cbegin(),
                        subcoordinators.cend(),
                        next_times.begin(),
                        [] (const auto& c) -> TIME { return c->next(); }
                );
                return *std::min_element(next_times.begin(), next_times.end());
            }

            template <typename TIME>
            static void collect_messages_by_eoc(const subcoordinators_type<TIME>& subcoordinators){
                //process one coupling
                std::vector<cadmium::dynamic::message_bags> outputs(subcoordinators.size());
                std::transform(
                        subcoordinators.cbegin(),
                        subcoordinators.cend(),
                        outputs.begin(),
                        [] (const auto& c) -> cadmium::dynamic::message_bags { return c->output(); }
                );


                auto from_bag = get_engine_by_model<submodel_from, CST>(cst).outbox();
                auto& from_messages = get_messages<submodel_output_port>(from_bag);
                auto& to_messages = get_messages<external_output_port>(messages);
                to_messages.insert(to_messages.end(), from_messages.begin(), from_messages.end());

//TODO(Lao): implement logs
                //log
//                auto log_routing_collect = [](decltype(from_messages) from, decltype(to_messages) to) -> std::string {
//                    std::ostringstream oss;
//                    oss << " in port ";
//                    oss << boost::typeindex::type_id<external_output_port>().pretty_name();
//                    oss << " has ";
//                    logger::implode(oss, to);
//                    oss << " routed from ";
//                    oss << boost::typeindex::type_id<submodel_output_port>().pretty_name();
//                    oss << " of model ";
//                    oss << boost::typeindex::type_id<submodel_from>().pretty_name();
//                    oss << " with messages ";
//                    logger::implode(oss, from);
//                    return oss.str();
//                };
//                LOGGER::template log<cadmium::logger::logger_message_routing,
            };
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_ENGINE_HELPERS_HPP
