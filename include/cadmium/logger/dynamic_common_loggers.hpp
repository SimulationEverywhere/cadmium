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

#ifndef CADMIUM_DYNAMIC_COMMON_LOGGERS_HPP
#define CADMIUM_DYNAMIC_COMMON_LOGGERS_HPP

#include <cadmium/engine/common_helpers.hpp>
#include <vector>
#include <string>

namespace cadmium {
    namespace dynamic {
        namespace logger {

            struct routed_messages {
                std::string from_port;
                std::string to_port;
                std::vector<std::string> from_messages;
                std::vector<std::string> to_messages;

                routed_messages() = default;

                routed_messages(
                        std::string from_p,
                        std::string to_p
                ) :
                        from_port(from_p),
                        to_port(to_p)
                {}

                routed_messages(
                        std::vector<std::string> from_msgs,
                        std::vector<std::string> to_msgs,
                        std::string from_p,
                        std::string to_p
                ) :
                        from_port(from_p),
                        to_port(to_p),
                        from_messages(from_msgs),
                        to_messages(to_msgs)
                {}

                routed_messages(const routed_messages& other) :
                        from_port(other.from_port),
                        to_port(other.to_port),
                        from_messages(other.from_messages),
                        to_messages(other.to_messages)
                {}

            };

            template<typename TIME>
            struct formatter {

                static std::string coor_info_init(const TIME& t, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "Coordinator for model ";
                    oss << model_id;
                    oss << " initialized to time ";
                    oss << t;
                    return oss.str();
                };

                static std::string coor_info_collect(const TIME& t, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "Coordinator for model ";
                    oss << model_id;
                    oss << " collecting output at time ";
                    oss << t;
                    return oss.str();
                };

                static std::string coor_routing_eoc_collect(const TIME& t, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "EOC for model ";
                    oss << model_id;
                    return oss.str();
                };

                static std::string coor_info_advance(const TIME& from, const TIME& to, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "Coordinator for model ";
                    oss << model_id;
                    oss << " advancing simulation from time ";
                    oss << from;
                    oss << " to ";
                    oss << to;
                    return oss.str();
                };

                static std::string coor_routing_ic_collect(const TIME& t, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "IC for model ";
                    oss << model_id;
                    return oss.str();
                };

                static std::string coor_routing_eic_collect(const TIME& t, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "EIC for model ";
                    oss << model_id;
                    return oss.str();
                };

                static std::string coor_routing_collect(const std::string& from_port, const std::string& to_port, const std::vector<std::string>& from_messages, const std::vector<std::string>& to_messages) {
                    std::ostringstream oss;
                    oss << " in port ";
                    oss << to_port;
                    oss << " has ";
                    oss << cadmium::helper::join(to_messages);
                    oss << " routed from ";
                    oss << from_port;
                    oss << " with messages ";
                    oss << cadmium::helper::join(from_messages);
                    return oss.str();
                }

                static std::string sim_info_init(const TIME& t, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "Simulator for model ";
                    oss << model_id;
                    oss << " initialized to time ";
                    oss << t;
                    return oss.str();
                }

                static std::string sim_state(const TIME& t, const std::string& model_id, const std::string& model_state) {
                    std::ostringstream oss;
                    oss << "State for model ";
                    oss << model_id;
                    oss << " is ";
                    oss << model_state;
                    return oss.str();
                };

                static std::string sim_info_collect(const TIME& t, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "Simulator for model ";
                    oss << model_id;
                    oss << " collecting output at time ";
                    oss << t;
                    return oss.str();
                };

                static std::string sim_messages_collect(const TIME& t, const std::string& model_id, const std::string& outbox) {
                    std::ostringstream oss;
                    oss << outbox;
                    oss << " generated by model ";
                    oss << model_id;
                    return oss.str();
                };

                static std::string sim_info_advance(const TIME& from, const TIME& to, std::string model_id) {
                    std::ostringstream oss;
                    oss << "Simulator for model ";
                    oss << model_id;
                    oss << " advancing simulation from time ";
                    oss << from;
                    oss << " to ";
                    oss << to;
                    return oss.str();
                };

                static std::string sim_local_time(const TIME& from, const TIME& to, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "Elapsed in model ";
                    oss << model_id;
                    oss << " is ";
                    oss << (to - from);
                    oss << "s";
                    return oss.str();
                };

                static TIME run_global_time(const TIME& global_time) {
                    return global_time;
                }

                static std::string run_info(const std::string& message) {
                    return message;
                }
            };
        }
    }
}

#endif //CADMIUM_DYNAMIC_COMMON_LOGGERS_HPP
