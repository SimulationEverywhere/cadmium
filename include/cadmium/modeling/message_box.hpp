/**
 * Copyright (c) 2019, Damian Vicino
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

#ifndef CADMIUM_MESSAGES_BOX_HPP
#define CADMIUM_MESSAGES_BOX_HPP

#include<optional>
#include<tuple>

/**
 * Here we declare the tools to manage messages in the context of DEVS models.
 * A message box is a set of ports that can receive optional messages in each port.
 */
namespace cadmium {
    template<typename PORT>
    struct message_box {
        using port=PORT;
        using message_type=typename PORT::message_type;

        std::optional<message_type> message;
    };

    template<typename... Ps>
    std::tuple<message_box<Ps>...> make_message_box_impl(std::tuple<Ps...>){
            return std::tuple<message_box<Ps>...>{};
    }

    template<typename T>
    struct make_message_box{
        using type=decltype(make_message_box_impl(T{}));
    };

    template<typename PORT, typename T>
    std::optional<typename message_box<PORT>::message_type> & get_message(T& mb){
        return std::get<message_box<PORT>>(mb).message;
    }

    template<typename PORT, typename T>
    const std::optional<typename message_box<PORT>::message_type> & get_message(const T& mb){
        return std::get<message_box<PORT>>(mb).message;
    }
}

#endif //CADMIUM_MESSAGE_BOX_HPP
