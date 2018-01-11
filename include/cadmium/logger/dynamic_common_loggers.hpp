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

#include <sstream>
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_model.hpp>

namespace cadmium {
    namespace dynamic {
        namespace logger {

            template<typename TIME>
            struct coordinator_formatter {

                static std::string log_info_init(const TIME& t, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "Coordinator for model ";
                    oss << model_id;
                    oss << " initialized to time ";
                    oss << t;
                    return oss.str();
                };

                static std::string log_info_collect(const TIME& t, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "Coordinator for model ";
                    oss << model_id;
                    oss << " collecting output at time ";
                    oss << t;
                    return oss.str();
                };

                static std::string log_routing_collect(const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "EOC for model ";
                    oss << model_id;
                    return oss.str();
                };

                static std::string log_info_advance(const TIME& from, const TIME& to, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "Coordinator for model ";
                    oss << model_id;
                    oss << " advancing simulation from time ";
                    oss << from;
                    oss << " to ";
                    oss << to;
                    return oss.str();
                };

                static std::string log_routing_ic_collect(const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "IC for model ";
                    oss << model_id;
                    return oss.str();
                };

                static std::string log_routing_eic_collect(const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "EIC for model ";
                    oss << model_id;
                    return oss.str();
                };
            };

            template <typename TIME>
            struct simulator_formatter {

                static std::string log_info_init(const TIME& t, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "Simulator for model ";
                    oss << model_id;
                    oss << " initialized to time ";
                    oss << t;
                    return oss.str();
                }

                static std::string log_state(const std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>>& model) {
                    std::ostringstream oss;
                    oss << "State for model ";
                    oss << model->get_id();
                    oss << " is ";
                    oss << model->model_state_as_string();
                    return oss.str();
                };

                static std::string log_info_collect(const TIME& t, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "Simulator for model ";
                    oss << model_id;
                    oss << " collecting output at time ";
                    oss << t;
                    return oss.str();
                };

                static std::string log_messages_collect(const std::string& model_id, const std::string& outbox) {
                    std::ostringstream oss;
                    oss << outbox;
                    oss << " generated by model ";
                    oss << model_id;
                    return oss.str();
                };

                static std::string log_info_advance(const TIME& from, const TIME& to, std::string model_id) {
                    std::ostringstream oss;
                    oss << "Simulator for model ";
                    oss << model_id;
                    oss << " advancing simulation from time ";
                    oss << from;
                    oss << " to ";
                    oss << to;
                    return oss.str();
                };

                static std::string log_local_time(const TIME& from, const TIME& to, const std::string& model_id) {
                    std::ostringstream oss;
                    oss << "Elapsed in model ";
                    oss << model_id;
                    oss << " is ";
                    oss << (to - from);
                    oss << "s";
                    return oss.str();
                };
            };
        }
    }
}

#endif //CADMIUM_DYNAMIC_COMMON_LOGGERS_HPP
