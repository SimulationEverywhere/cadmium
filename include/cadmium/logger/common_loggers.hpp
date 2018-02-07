/**
 * Copyright (c) 2013-2018, Damian Vicino, Laouen M. L. Belloli
 * Carleton University, Universite de Nice-Sophia Antipolis, Universidad de Buenos Aires
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

#ifndef COMMON_LOGGERS_HPP
#define COMMON_LOGGERS_HPP

#include <cadmium/logger/logger.hpp>
#include <cadmium/logger/common_loggers_helpers.hpp>

namespace cadmium {
    namespace logger {

        //Commmon sink providers
        struct cout_sink_provider{
            static std::ostream& sink(){
                return std::cout;
            }
        };
        struct cerr_sink_provider{
            static std::ostream& sink(){
                return std::cerr;
            }
        };

        //traits for helping with the verbatim formatter.
        template<typename...>
        using void_t = void; //until C++17 is around

        template<typename, typename = void>
        struct is_callable : std::false_type {};

        template<typename F, typename... Args>
        struct is_callable<F(Args...), void_t<decltype(std::declval<F>()(std::declval<Args>()...))>> : std::true_type {};

        template<typename E>
        constexpr auto is_callable_v = is_callable<E>::value;

        //Common formatters
        /** verbatim_formater
          *
          * if first param is callable it is used to convert all other params into streamable content
          * else introduces every param into the stream
          */

        //formatter for verbatim, takes a function in first param and applies it to all other params
        // or concatenated outputs
        struct verbatim_formatter {
            template<typename F, typename... Args>
            static auto format(std::ostream& os, F func, Args&&... args) -> std::enable_if_t<is_callable_v<F(Args...)>> {
                os << func(std::forward<Args>(args)...);
                os << std::endl;
            }


            template<typename T, typename... TS>
            static auto format(std::ostream& os, T&& value, TS&&... ts) -> std::enable_if_t<!is_callable_v<T(TS...)>> {
                os << std::forward<T>(value);
                verbatim_formatter::format(os, std::forward<TS>(ts)...);
            }

           static void format(std::ostream& os){
                os << std::endl;
           }
        };


        //a logger that should not match any source, mostly for testing and debug purposes
        struct not_matching_source :public logger_source{};
        using not_logger=logger<not_matching_source, verbatim_formatter, cout_sink_provider>;

        template<typename TIME>
        struct formatter {

            static std::string coor_info_init(TIME t, const std::string& model_id) {
                std::ostringstream oss;
                oss << "Coordinator for model ";
                oss << model_id;
                oss << " initialized to time ";
                oss << t;
                return oss.str();
            };

            static std::string coor_info_collect(TIME t, const std::string& model_id) {
                std::ostringstream oss;
                oss << "Coordinator for model ";
                oss << model_id;
                oss << " collecting output at time ";
                oss << t;
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

            static std::string coor_routing_eoc_collect(const std::string& model_id) {
                std::ostringstream oss;
                oss << "EOC for model ";
                oss << model_id;
                return oss.str();
            };

            static std::string coor_routing_ic_collect(const std::string& model_id) {
                std::ostringstream oss;
                oss << "IC for model ";
                oss << model_id;
                return oss.str();
            };

            static std::string coor_routing_eic_collect(const std::string& model_id) {
                std::ostringstream oss;
                oss << "EIC for model ";
                oss << model_id;
                return oss.str();
            };

            static std::string coor_routing_collect_eoc(const std::string& from_messages, const std::string& to_messages, const std::string& from_port, const std::string& to_port, const std::string& submodel_from) {
                std::ostringstream oss;
                oss << " in port ";
                oss << to_port;
                oss << " has ";
                oss << to_messages;
                oss << " routed from ";
                oss << from_port;
                oss << " of model ";
                oss << submodel_from;
                oss << " with messages ";
                oss << from_messages;
                return oss.str();
            };

            static std::string coor_routing_collect_ic(const std::string& from_messages, const std::string& to_messages, const std::string& from_port, const std::string& from_model, const std::string& to_port, const std::string& to_model) {
                std::ostringstream oss;
                oss << " in port ";
                oss << to_port;
                oss << " of model ";
                oss << to_model;
                oss << " has ";
                oss << to_messages;
                oss << " routed from ";
                oss << from_port;
                oss << " of model ";
                oss << from_model;
                oss << " with messages ";
                oss << from_messages;
                return oss.str();
            };

            static std::string coor_routing_collect_eic(const std::string& from_messages, const std::string& to_messages, const std::string& to_port, const std::string& to_model, const std::string& from_port) {
                std::ostringstream oss;
                oss << " in port ";
                oss << to_port;
                oss << " of model ";
                oss << to_model;
                oss << " has ";
                oss << to_messages;
                oss << " routed from ";
                oss << from_port;
                oss << " with messages ";
                oss << from_messages;
                return oss.str();
            };

            static std::string sim_info_init(TIME t, const std::string& model_id) {
                std::ostringstream oss;
                oss << "Simulator for model ";
                oss << model_id;
                oss << " initialized to time ";
                oss << t;
                return oss.str();
            }

            static std::string sim_state(const std::string& state, const std::string& model_id) {
                std::ostringstream oss;
                oss << "State for model ";
                oss << model_id;
                oss << " is ";
                oss << state;
                return oss.str();
            };

            static std::string sim_info_collect(TIME t, const std::string& model_id) {
                std::ostringstream oss;
                oss << "Simulator for model ";
                oss << model_id;
                oss << " collecting output at time ";
                oss << t;
                return oss.str();
            };

            static std::string sim_messages_collect(const std::string& messages_by_port, const std::string& model_id) {
                std::ostringstream oss;
                oss << messages_by_port;
                oss << " generated by model ";
                oss << model_id;
                return oss.str();
            };

            static std::string sim_info_advance(const TIME& from, const TIME& to, const std::string& model_id) {
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


#endif // COMMON_LOGGERS_HPP
