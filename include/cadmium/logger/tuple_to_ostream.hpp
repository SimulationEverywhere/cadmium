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

#ifndef TUPLE_TO_OSTREAM_HPP
#define TUPLE_TO_OSTREAM_HPP

/**
 * @brief Here we define a simple helper function for output tuple to ostream
 * The tuple is iterated, and each component outputs its own value to ostream
 * We use brackets to contain the values and separate them by commas
 *
 * This is not a safe method, its intended as quick workaround for displaying
 * states and messages that are defined as tuple for logging purposes.
 * This is not included by the simulator, needs to be included explicitely by the user
 * The reason is that
 */

#include <tuple>
#include <iostream>

namespace cadmium {
    template<size_t s, typename... T>
    struct tuple_printer{
        static void print(std::ostream& os, const std::tuple<T...>& t){
            tuple_printer<s-1, T...>::print(os, t);
            os << ", ";
            os << std::get<s-1>(t);
        }
    };

    template<typename... T>
    struct tuple_printer<0, T...>{
        static void print(std::ostream& os, const std::tuple<T...>& t){
            //nothing to do here
        }
    };

    template<typename... T>
    struct tuple_printer<1, T...>{
        static void print(std::ostream& os, const std::tuple<T...>& t){
            //no need for comma separator
            os << std::get<0>(t);
        }
    };

    template <typename... T>
    std::ostream& operator<<(std::ostream& os, const std::tuple<T...>& t){
        os << "[";
        tuple_printer<sizeof...(T), T...>::print(os, t);
        return os << "]";
    }

}
#endif // TUPLE_TO_OSTREAM_HPP
