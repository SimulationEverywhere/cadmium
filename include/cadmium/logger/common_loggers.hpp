/**
 * Copyright (c) 2013-2017, Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis
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

namespace cadmium {
    namespace logger {
        //Common source identifiers
        struct logger_info : public cadmium::logger::logger_source{};
        struct logger_debug : public cadmium::logger::logger_source{};
        struct logger_state : public cadmium::logger::logger_source{};
        struct logger_messages : public cadmium::logger::logger_source{};
        struct logger_global_time : public cadmium::logger::logger_source{};
        struct logger_local_time :  public cadmium::logger::logger_source{};

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

        //Common formatters
        //formatter for verbatim concatenated outputs
        template<typename... PARAMs>
        struct verbatim_formater;

        template<typename PARAM, typename... PARAMs>
        struct verbatim_formater<PARAM, PARAMs...>{
            static void format(std::ostream& os, const PARAM& p, const PARAMs&... ps){
                os << p;
                verbatim_formater<PARAMs...>::format(os, ps...);
            }
        };

        template<>
        struct verbatim_formater<>{
            static void format(std::ostream& os){
                os << std::endl;
            }
        };
    }
}


#endif // COMMON_LOGGERS_HPP
