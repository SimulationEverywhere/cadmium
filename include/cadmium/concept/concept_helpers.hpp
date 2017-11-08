/**
 * Copyright (c) 2013-2015, Damian Vicino
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

#ifndef CADMIUM_HELPERS_HPP
#define CADMIUM_HELPERS_HPP

#include<tuple>

namespace cadmium {
    namespace concept {


        template<template<typename> class MODEL>
        struct is_atomic{
            struct atomic_detected{};
            struct coupled_detected{};

            template <typename M> static coupled_detected test( typename M::template models<float>* );
            template <typename M> static atomic_detected test( ...  ) ;

            static constexpr bool value(){
                return std::is_same<decltype(test<MODEL<float>>(0)), atomic_detected>::value;
            }
        };

        namespace { //details
            template<typename, template<typename...> class>
            struct is_specialization : std::false_type {};
            template<template<typename...> class TEMP, typename... ARGS>
            struct is_specialization<TEMP<ARGS...>, TEMP>: std::true_type {};
            
            template<typename T>
            constexpr bool is_tuple(){
                return is_specialization<T, std::tuple>::value;
            }
            
            template<typename T, int S>
            struct check_unique_elem_types_impl {
                static constexpr bool value() {
                    using elem=typename std::tuple_element<S - 1, T>::type;
                    std::get<elem>(T{});
                    return check_unique_elem_types_impl<T, S - 1>::value();
                }
            };

            template<typename T>
            struct check_unique_elem_types_impl<T, 0> {
                static constexpr bool value() {
                    return true;
                }
            };

            template<typename T>
            struct check_unique_elem_types {
                static constexpr bool value() {
                    return check_unique_elem_types_impl<T, std::tuple_size<T>::value>::value();
                }
            };



            template<typename PORT, typename TUPLE, int S>
            struct has_port_in_tuple_impl{
                static constexpr bool value(){
                    if (std::is_same<PORT, typename std::tuple_element<S-1, TUPLE>::type>()){
                        return true;
                    } else {
                        return has_port_in_tuple_impl<PORT, TUPLE, S-1>::value();
                    }
                }
            };

            template<typename PORT, typename TUPLE>
            struct has_port_in_tuple_impl<PORT, TUPLE, 0>{
                static constexpr bool value(){
                    return false;
                }
            };

            template<typename PORT, typename TUPLE>
            struct has_port_in_tuple{
                static constexpr bool value(){
                    return has_port_in_tuple_impl<PORT, TUPLE, std::tuple_size<TUPLE>::value>::value();
                }
            };

        }
    }
}
#endif // CADMIUM_HELPERS_HPP

