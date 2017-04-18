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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <sstream>
#include <iostream>

/**
  * Logging concepts
  * A logger source is where information of potential interest is provided to the logger.
  *   Syntax is similar to IOstream using operator<<.
  *   In addition to the information to log, a set of identifiers is provided to decide where to route it.
  * A filter makes decisions, based in the set of identifiers provided by the log source, to effectively log the information.
  *   We filter at compile time to reduce any undecired logging overhead.
  * A formatter transforms the information that was not filtered out into a log record.
  * A sink provides the way to move formated log records to its final destination.
  *
  * Expected usage
  *   This logger is not a generic implementation to use in any system. In the contrary, it tries to exploit
  *   particularities of Cadmium simulator to reduce overhead.
  *
  * Assumptions at this stage
  *   All log sources are well-known (runner, coordinator, simulator), and there is not point of extension.
  *   Each introduction of log source information, if not filtered out, will produce one log record.
  *   All possible identifier are known.
  *
  * Future additions
  *   We will like to allow an extension for model specific log sources and filters.
  */


namespace cadmium {
    namespace logger {
        //log sources are identified as childs of this class
        struct logger_source{};

        template<typename LOGGER_SOURCE, class FORMATTER, typename SINK_PROVIDER>
        struct logger{
            template<typename DECLARED_SOURCE, typename... PARAMs>
            static void log(const PARAMs&... ps){ //todo: apply a static_if solution
                if (std::is_same<LOGGER_SOURCE, DECLARED_SOURCE>::value) {
                    FORMATTER::format(SINK_PROVIDER::sink(), ps...);
                }
            }
        };

        template<typename... LS>
        struct multilogger_impl;

        template<typename L, typename... LS>
        struct multilogger_impl<L, LS...>{
            template<typename DECLARED_SOURCE, typename... PARAMs>
            static void log(const PARAMs&... ps){
                L::template log<DECLARED_SOURCE, PARAMs...>(ps...);
                //recurse
                multilogger_impl<LS...>::template log<DECLARED_SOURCE, PARAMs...>(ps...);
            }
        };

        template<>
        struct multilogger_impl<>{
            template<typename DECLARED_SOURCE, typename... PARAMs>
            static void log(const PARAMs&... ps){
                //nothing to do
            }
        };


        template<typename... LS>
        struct multilogger{
            template<typename DECLARED_SOURCE, typename... PARAMs>
            static void log(const PARAMs&... ps){
                multilogger_impl<LS...>::template log<DECLARED_SOURCE, PARAMs...>(ps...);
            }
        };
    }
}

#endif // LOGGER_HPP
