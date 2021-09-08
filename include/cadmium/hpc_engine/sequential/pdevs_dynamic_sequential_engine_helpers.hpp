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

#ifndef CADMIUM_PDEVS_DYNAMIC_SEQUENTIAL_ENGINE_HELPERS_HPP
#define CADMIUM_PDEVS_DYNAMIC_SEQUENTIAL_ENGINE_HELPERS_HPP

#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/common_loggers.hpp>
//#include <algorithm>
#include <cadmium/engine/pdevs_dynamic_link.hpp>

#include <cadmium/hpc_engine/sequential/pdevs_dynamic_sequential_engine.hpp>
#include <cadmium/hpc_engine/sequential/pdevs_dynamic_sequential_simulator.hpp>

namespace cadmium {
    namespace dynamic {
        namespace hpc_engine {
            namespace sequential {
                //Checking all dynamic bag of inbox or outbox are empty
                template<class BOX>
                decltype(auto) all_bags_empty(cadmium::dynamic::message_bags const& dynamic_bag) {
                    auto is_empty = [&dynamic_bag](auto b) -> bool {
                        using bag_type = decltype(b);
                        using port_type = typename bag_type::port;

                        if (dynamic_bag.find(typeid(port_type)) != dynamic_bag.cend()) {
                            return boost::any_cast<bag_type>(dynamic_bag.at(typeid(port_type))).messages.empty();
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

                template<typename TIME, typename LOGGER>
                using subcoordinators_type = typename std::vector<std::shared_ptr<cadmium::dynamic::hpc_engine::sequential::sequential_simulator<TIME, LOGGER>>>;
                using external_port_couplings = typename std::map<std::string, std::vector<std::shared_ptr<cadmium::dynamic::engine::link_abstract>>>;

                template<typename TIME, typename LOGGER>
                using external_couplings = typename std::vector<
                        std::pair<
                                std::shared_ptr<cadmium::dynamic::hpc_engine::sequential::sequential_simulator<TIME, LOGGER>>,
                                std::vector<std::shared_ptr<cadmium::dynamic::engine::link_abstract>>
                        >
                >;

                template<typename TIME, typename LOGGER>
                using internal_coupling = std::pair<
                        std::pair<
                                std::shared_ptr<cadmium::dynamic::hpc_engine::sequential::sequential_simulator<TIME, LOGGER>>, // from model
                                std::shared_ptr<cadmium::dynamic::hpc_engine::sequential::sequential_simulator<TIME, LOGGER>> // to model
                        >,
                        std::vector<std::shared_ptr<cadmium::dynamic::engine::link_abstract>>
                >;

                template<typename TIME, typename LOGGER>
                using internal_couplings = typename std::vector<internal_coupling<TIME, LOGGER>>;

                template<typename TIME, typename LOGGER>
                using external_coupling = std::pair<
                        std::shared_ptr<cadmium::dynamic::hpc_engine::sequential::sequential_simulator<TIME, LOGGER>>,
                        std::vector<std::shared_ptr<cadmium::dynamic::engine::link_abstract>>
				>;

                template<typename TIME, typename LOGGER>
                void init_subcoordinators(TIME t, subcoordinators_type<TIME, LOGGER>& subcoordinators) {
                    auto init_coordinator = [&t](auto & c)->void { c->init(t); };
                    std::for_each(subcoordinators.begin(), subcoordinators.end(), init_coordinator);
                }

                template<typename TIME, typename LOGGER>
                void state_transition_in_subengines(TIME t, subcoordinators_type<TIME, LOGGER>& subcoordinators) {
                   auto advance_time= [&t](auto &c)->void { c->state_transition(t); };
                    std::for_each(subcoordinators.begin(), subcoordinators.end(), advance_time);
                }

                template<typename TIME, typename LOGGER>
                void collect_outputs_in_subcoordinators(TIME t, subcoordinators_type<TIME, LOGGER>& subcoordinators) {
                    auto collect_output = [&t](auto & c)->void { c->collect_outputs(t); };
                    std::for_each(subcoordinators.begin(), subcoordinators.end(), collect_output);
                }

                template<typename TIME, typename LOGGER>
                cadmium::dynamic::message_bags collect_messages_by_eoc(const external_couplings<TIME, LOGGER>& coupling) {
                    cadmium::dynamic::message_bags ret;
                    auto collect_output = [&ret](auto & c)->void {
                        cadmium::dynamic::message_bags outbox = c.first->outbox();
                        for (const auto& l : c.second) {
                            cadmium::dynamic::logger::routed_messages message_to_log = l->route_messages(outbox, ret);

                            LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_collect>(message_to_log.from_port, message_to_log.to_port, message_to_log.from_messages, message_to_log.to_messages);
                        }
                    };

                    std::for_each(coupling.begin(), coupling.end(), collect_output);
                    return ret;
                }

                template<typename TIME, typename LOGGER>
                void route_external_input_coupled_messages_on_subcoordinators(cadmium::dynamic::message_bags inbox, const external_couplings<TIME, LOGGER>& coupling) {
                    auto route_messages = [&inbox](auto & c)->void {
                        for (const auto& l : c.second) {
                            auto& to_inbox = c.first->inbox();
                            cadmium::dynamic::logger::routed_messages message_to_log = l->route_messages(inbox, to_inbox);

                            LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_collect>(message_to_log.from_port, message_to_log.to_port, message_to_log.from_messages, message_to_log.to_messages);
                        }
                    };

                    std::for_each(coupling.begin(), coupling.end(), route_messages);
                }

                template<typename TIME, typename LOGGER>
                void route_internal_coupled_messages_on_subcoordinators(const internal_couplings<TIME, LOGGER>& coupling) {
                    auto route_messages = [](auto & c)->void {
                	    for (const auto& l : c.second) {
                            auto& from_outbox = c.first.first->outbox();
                            auto& to_inbox = c.first.second->inbox();
                            cadmium::dynamic::logger::routed_messages message_to_log = l->route_messages(from_outbox, to_inbox);

                            LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_collect>(message_to_log.from_port, message_to_log.to_port, message_to_log.from_messages, message_to_log.to_messages);
                		}
                    };

                	std::for_each(coupling.begin(), coupling.end(), route_messages);
                }

                template<typename TIME, typename LOGGER>
                TIME min_next_in_subcoordinators(const subcoordinators_type<TIME, LOGGER>& subcoordinators) {
                    std::vector<TIME> next_times(subcoordinators.size());
                    std::transform(
                            subcoordinators.cbegin(),
                            subcoordinators.cend(),
                            next_times.begin(),
                            [] (const auto& c) -> TIME { return c->next(); }
                    );
                    return *std::min_element(next_times.begin(), next_times.end());
                }
            }
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_ENGINE_HELPERS_HPP
