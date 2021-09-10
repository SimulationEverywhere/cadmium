/**
 * Copyright (c) 2020, Guillermo G. Trabes
 * Carleton University, Universidad Nacional de San Luis
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

#ifndef CADMIUM_PARALLEL_ENGINE_HELPERS_HPP
#define CADMIUM_PARALLEL_ENGINE_HELPERS_HPP

#include <vector>
#include <functional>
#include <chrono>
#include <omp.h>
#include <thread>
#include <algorithm>
#include <array>

#include <cadmium/logger/common_loggers.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <iostream>
#include <sched.h>
#include <vector>
#include <utility>
#include <tuple>

namespace cadmium {
    namespace dynamic {
        namespace hpc_engine {

            template<typename TIME>
    	    struct info_for_logging {
    		    std::string type;
    		    TIME time;
    		    TIME last;
    		    bool imminent;
    		    bool imminent_or_receiver;
			    std::string model_id;
    		    std:: string messages_by_port;
    		    std::string state_as_string;
            };

    	    void pin_thread_to_core(size_t tid){
    		    size_t len, core;
    		    cpu_set_t mascara;
    		    CPU_ZERO (&mascara);
    		    size_t thread_number = std::thread::hardware_concurrency();

    		    //set thread to tid core % number of threads
    		    len = sizeof(cpu_set_t);
    		    core = tid % thread_number;
    		    CPU_SET (core, &mascara);
    		    if (sched_setaffinity(0, len, &mascara) < 0)
    			    printf("\n\nError :: sched_setaffinity\n\n");
            }

    	    void pin_thread_to_core_xeon(size_t tid){
    		    size_t len, core;
    		    cpu_set_t mascara;
    		    CPU_ZERO (&mascara);
    		    size_t thread_number = std::thread::hardware_concurrency();

    		    //set thread to tid core % number of threads
    		    len = sizeof(cpu_set_t);
    		    core = tid % thread_number;

    		    if(tid % thread_number == 0){
    		        core = 0;
    		    }

    		    if(tid % thread_number == 1){
    		        core = 1;
    		    }

    		    if(tid % thread_number == 2){
    		        core = 2;
    	        }

    		    if(tid % thread_number == 3){
    		        core = 3;
    	        }

    		    if(tid % thread_number == 4){
    		    	core = 8;
    		    }

    		    if(tid % thread_number == 5){
    		    	core = 9;
    		    }

    		    if(tid % thread_number == 6){
    		    	core = 10;
    	        }

    		    if(tid % thread_number == 7){
    		    	core = 11;
    	        }

    		    if(tid % thread_number == 8){
    		    	core = 4;
    		    }

    		    if(tid % thread_number == 9){
    		    	core = 5;
    		    }

    		    if(tid % thread_number == 10){
    		    	core = 6;
    	        }

    		    if(tid % thread_number == 11){
    		    	core = 7;
    	        }

    		    if(tid % thread_number == 12){
    		    	core = 12;
    		    }

    		    if(tid % thread_number == 13){
    		    	core = 13;
    		    }

    		    if(tid % thread_number == 14){
    		    	core = 14;
    	        }

    		    if(tid % thread_number == 15){
    		    	core = 15;
    	        }

    		    CPU_SET (core, &mascara);
    		    if (sched_setaffinity(0, len, &mascara) < 0)
    			    printf("\n\nError :: sched_setaffinity\n\n");
            }



    	}
    }
}

#endif //CADMIUM_PARALLEL_HELPERS_HPP
