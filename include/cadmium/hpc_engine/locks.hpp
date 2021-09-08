/**
 * Copyright (c) 2018, Laouen M. L. Belloli
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

#ifndef CADMIUM_PDEVS_DYNAMIC_LOGS_LOCK_HPP
#define CADMIUM_PDEVS_DYNAMIC_LOGS_LOCK_HPP

#include <typeindex>
#include <memory>

#include <cadmium/logger/dynamic_common_loggers.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/common_loggers_helpers.hpp>
#include <omp.h>

#include <limits.h>


namespace cadmium {
    namespace dynamic {
        namespace parallel_engine {

            class locks {
            private:
                static omp_lock_t route_lock[INT_MAX];
                static omp_lock_t logs_lock;

                locks() {}

                ~locks(){
                    omp_destroy_lock(&logs_lock);
                    for(size_t i=0; i<INT_MAX; i++){
                        omp_destroy_lock(&route_lock[i]);
                    }
                }

            public:
                void init_locks() {
                    omp_init_lock(&logs_lock);
                    for(size_t i=0; i<INT_MAX; i++){
                        omp_init_lock(&route_lock[i]);
                    }
                }

                void set_route_lock(size_t index) {
                    omp_set_lock(&route_lock[index]);
                }

                void set_log_lock() {
                    omp_set_lock(&logs_lock);
                }

                void release_route_lock(size_t index) {
                    omp_set_lock(&route_lock[index]);
                }

                void release_log_lock() {
                    omp_set_lock(&logs_lock);
                }

            };
        }
    }
}


#endif //CADMIUM_PDEVS_DYNAMIC_LINK_HPP
