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
        namespace hpc_engine {
            namespace cpu_parallel {
                static std::map<std::type_index, omp_lock_t> route_locks;
                static omp_lock_t logs_lock;

 //               class locks {
 //               public:
                    //locks(){};
/*
                    ~locks(){
                        omp_destroy_lock(&logs_lock);
                        for(size_t i=0; i<INT_MAX; i++){
                            omp_destroy_lock(&route_lock[i]);
                        }
                    }
*/
/*
                    void init_lock() {
                        omp_init_lock(&logs_lock);
                        for(size_t i=0; i<INT_MAX; i++){
                            omp_init_lock(&route_lock[i]);
                        }
                    }
*/
                    static void set_route_lock(std::type_index index) {
//                        if (cadmium::dynamic::hpc_engine::cpu_parallel::route_locks.find(index) != cadmium::dynamic::hpc_engine::cpu_parallel::route_locks.cend()) {
//                            omp_set_lock(&cadmium::dynamic::hpc_engine::cpu_parallel::route_locks[index]);
//                        }
//                        else {
//                            omp_lock_t lock;
//                            cadmium::dynamic::hpc_engine::cpu_parallel::route_locks[index] = lock;
//                            omp_init_lock(&(cadmium::dynamic::hpc_engine::cpu_parallel::route_locks[index]));
//                            omp_set_lock(&(cadmium::dynamic::hpc_engine::cpu_parallel::route_locks[index]));
//                        }
                    }

                    static void set_log_lock() {
                        omp_set_lock(&(cadmium::dynamic::hpc_engine::cpu_parallel::logs_lock));
                    }

                    static void release_route_lock(std::type_index index) {
//                        omp_unset_lock(&cadmium::dynamic::hpc_engine::cpu_parallel::route_locks[index]);
                    }

                    static void release_log_lock() {
                        omp_unset_lock(&(cadmium::dynamic::hpc_engine::cpu_parallel::logs_lock));
                    }

//                };
            }
        }
    }
}


#endif //CADMIUM_PDEVS_DYNAMIC_LINK_HPP
