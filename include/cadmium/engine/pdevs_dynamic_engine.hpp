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

#ifndef CADMIUM_PDEVS_DYNAMIC_ENGINE_HPP
#define CADMIUM_PDEVS_DYNAMIC_ENGINE_HPP

#include <cadmium/modeling/dynamic_message_bag.hpp>

#ifdef CADMIUM_EXECUTE_CONCURRENT
#include <boost/thread/executors/basic_thread_pool.hpp>
#endif

namespace cadmium {
    namespace dynamic {
        namespace engine {

            /**
             * @brief Abstract class to allow pointer polymorphism between dynamic::coordinator
             * and dynamic::atomic
             *
             * @tparam TIME
             */
            template<typename TIME>
            class engine {
            public:
                virtual void init(TIME initial_time) = 0;

                #ifdef CADMIUM_EXECUTE_CONCURRENT
                virtual void init(TIME initial_time, boost::basic_thread_pool* threadpool) = 0;
                #endif

                virtual std::string get_model_id() const = 0;

                virtual TIME next() const noexcept = 0;

                virtual void collect_outputs(const TIME &t) = 0;

                virtual cadmium::dynamic::message_bags& outbox() = 0;

                virtual cadmium::dynamic::message_bags& inbox() = 0;

                virtual void advance_simulation(const TIME &t) = 0;
            };
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_ENGINE_HPP
