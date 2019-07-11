/**
 * Copyright (c) 2013-2019, Damian Vicino
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

#ifndef CADMIUM_MESSAGE_BAG_HPP
#define CADMIUM_MESSAGE_BAG_HPP

#include <vector>
#include <tuple>
#include <typeindex>
#include <map>

/**
 * Here we declare the tools to manage messages in the context of PDEVS models.
 * A message bag is a set of messages that can be received in a port, and multiple ports
 * can receive messages at the same time.
 */


namespace cadmium {

template<typename T>
using bag=std::vector<T>;

template<typename PORT>
struct message_bag{
    using port=PORT;
    using message_type=typename PORT::message_type;

    bag<message_type> messages;

    message_bag(){}

    message_bag(std::initializer_list<message_type> l) : messages{l} {}
};

template<typename... Ps>
std::tuple<message_bag<Ps>...> make_message_bags_impl(std::tuple<Ps...>){
    return std::tuple<message_bag<Ps>...>{};
}

template<typename T>
struct make_message_bags{
    using type=decltype(make_message_bags_impl(T{}));
};


template<typename PORT, typename T>
bag<typename message_bag<PORT>::message_type> & get_messages(T& mbs){
    return std::get<message_bag<PORT>>(mbs).messages;
}

template<typename PORT, typename T>
const bag<typename message_bag<PORT>::message_type> & get_messages(const T& mbs){
    return std::get<message_bag<PORT>>(mbs).messages;
}

}

#endif // CADMIUM_MESSAGE_BAG_HPP

