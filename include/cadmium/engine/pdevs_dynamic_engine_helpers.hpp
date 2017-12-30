/**
 * Copyright (c) 2017, Laouen M. L. Belloli
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

#ifndef CADMIUM_PDEVS_DYNAMIC_ENGINE_HELPERS_HPP
#define CADMIUM_PDEVS_DYNAMIC_ENGINE_HELPERS_HPP

#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <boost/any.hpp>

namespace cadmium {
    namespace dynamic {
        namespace engine {
            //Checking all dynamic bag of inbox or outbox are empty
            template<class BOX>
            decltype(auto) all_bags_empty(cadmium::dynamic::message_bags const& dynamic_bag) {
                auto is_empty = [&dynamic_bag](auto const& b) -> bool {
                    using bag_type = decltype(b);
                    if (dynamic_bag.find(typeid(b)) != dynamic_bag.cend()) {
                        return boost::any_cast<bag_type>(dynamic_bag.at(typeid(b))).messages.empty();
                    }
                    // A not declared bag in the dynamic_bag is the same as a bag with empty messages
                    return true;
                };
                auto check_empty = [&is_empty](auto const&... b)->decltype(auto) {
                    return (is_empty(b) && ...);
                };

                // the box is used to get all the port types to use as keys.
                BOX box;
                return std::apply(check_empty, box);
            }
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_ENGINE_HELPERS_HPP
