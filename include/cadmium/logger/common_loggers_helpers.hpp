/**
 * Copyright (c) 2017, Damian Vicino, Laouen M. L. Belloli
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

#ifndef CADMIUM_COMMON_LOGGERS_HELPERS_HPP
#define CADMIUM_COMMON_LOGGERS_HELPERS_HPP

#include <tuple>
#include <boost/type_index.hpp>
#include <iostream>
#include <cadmium/modeling/message_bag.hpp>

namespace cadmium {
    namespace logger {

        // Displaying all messages in a bag
        //printing all messages in bags, if the support the << operator to ostream
        template <typename T>
        struct is_streamable {
        private:
            template <typename U>
            static decltype(std::cout << std::declval<U>(), void(), std::true_type()) test(int);
            template <typename>
            static std::false_type test(...);
        public:
            using type=decltype(test<T>(0));
            static constexpr auto value=type::value;
        };

        template<typename T, typename V=typename is_streamable<T>::type>
        struct value_or_name;

        template<typename T>
        struct value_or_name<T, std::true_type>{
            static void print(std::ostream& os, const T& v){
                os << v;
            }
        };

        template<typename T>
        struct value_or_name<T, std::false_type>{
            static void print(std::ostream& os, const T& v){
                os << "obscure message of type ";
                os << boost::typeindex::type_id<T>().pretty_name();
            }
        };

        template<typename T>
        std::ostream& implode(std::ostream& os, const T& collection){
            os << "{";
            auto it = std::begin(collection);
            if (it != std::end(collection)) {
                value_or_name<typename T::value_type>::print(os, *it);
                ++it;
            }
            while (it != std::end(collection)){
                os << ", ";
                value_or_name<typename T::value_type>::print(os, *it);
                ++it;
            }
            os << "}";
            return os;
        }

        template<typename T>
        std::vector<std::string> messages_as_strings(const T& collection){
            std::vector<std::string> ret;
            std::ostringstream oss;


            for (auto it = std::begin(collection); it != std::end(collection); ++it){
                value_or_name<typename T::value_type>::print(oss, *it);
                ret.push_back(oss.str());
                oss.str("");
                oss.clear();
            }
            return ret;
        }

        //priting messages for all ports
        template<size_t s, typename... T>
        struct print_messages_by_port_impl{
            using current_bag=typename std::tuple_element<s-1, std::tuple<T...>>::type;
            static void run(std::ostream& os, const std::tuple<T...>& b){
                print_messages_by_port_impl<s-1, T...>::run(os, b);
                os << ", ";
                os << boost::typeindex::type_id<typename current_bag::port>().pretty_name();
                os << ": ";
                implode(os, cadmium::get_messages<typename current_bag::port>(b));
            }
        };

        template<typename... T>
        struct print_messages_by_port_impl<1, T...>{
            using current_bag=typename std::tuple_element<0, std::tuple<T...>>::type;
            static void run(std::ostream& os, const std::tuple<T...>& b){
                os << boost::typeindex::type_id<typename current_bag::port>().pretty_name();
                os << ": ";
                implode(os, cadmium::get_messages<typename current_bag::port>(b));
            }
        };

        template<typename... T>
        struct print_messages_by_port_impl<0, T...>{
            static void run(std::ostream& os, const std::tuple<T...>& b){}
        };

        template <typename... T>
        void print_messages_by_port(std::ostream& os, const std::tuple<T...>& b){
            os << "[";
            print_messages_by_port_impl<sizeof...(T), T...>::run(os, b);
            os << "]";
        }
    }
}
#endif //CADMIUM_COMMON_LOGGERS_HELPERS_HPP
