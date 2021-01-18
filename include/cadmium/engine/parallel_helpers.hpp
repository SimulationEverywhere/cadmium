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

#ifndef CADMIUM_PARALLEL_HELPERS_HPP
#define CADMIUM_PARALLEL_HELPERS_HPP

#include <vector>
#include <functional>
#include <chrono>
#include <omp.h>
#include <thread>
#include <algorithm>
#include <array>
#include <typeinfo>

#include <cadmium/logger/common_loggers.hpp>

namespace cadmium {
	namespace parallel {

		void pin_thread_to_core(size_t tid){
			unsigned long len;
			cpu_set_t mascara;
			CPU_ZERO (&mascara);

			//size_t pin_to = tid % thread_number
			//set Thread to tid core
			len = sizeof(cpu_set_t);
			CPU_SET (tid, &mascara);
			if (sched_setaffinity(0, len, &mascara) < 0)
				printf("\n\nError :: sched_setaffinity\n\n");
		}

    	template<typename TIME>
    	struct info_for_logging {
    		std::string type;
    		TIME time;
    		TIME last;
    		bool imminent;
			std::string model_id;
    		std:: string messages_by_port;
    		std::string state_as_string;

    	};

    	template<typename ITERATOR, typename FUNC>
    	void cpu_parallel_for_each(ITERATOR first, ITERATOR last, FUNC& f, size_t thread_number = std::thread::hardware_concurrency()){
    		/* get amount of elements to compute */
    		size_t n = std::distance(first, last);
    		/* if the amount of elements is less than the number of threads execute on n elements */
    		if(n<thread_number){
    			thread_number=n;
    		}
    		/* OpenMP parallel loop */
    		/* It doesn't work -> execution time as sequential version
    		#pragma omp parallel for num_threads(thread_number) firstprivate(f, first) proc_bind(close) schedule(static)
    		for(int i = 0; i < n; i++){
    			f(*(i+first));
    		}
    		*/
			#pragma omp parallel firstprivate(f) num_threads(thread_number) proc_bind(close)
    		{
    			/* get thread id */
    			size_t tid = omp_get_thread_num();

    			/* if it's not last thread compute n/thread_number elements */
    			if(tid != thread_number-1) {
    				for(size_t i = (n/thread_number)*tid; i < (n/thread_number)*(tid+1); i++) {
    					f(*(i+first));
    				}
    			/* if it's last thread compute till the end of the vector */
				} else {
					for(size_t i = (n/thread_number) * tid; i < n ; i++) {
						f(*(i+first));
					}
				}
    		}
    	}

    	template<typename TIME, typename ITERATOR, typename FUNC>
    	std::vector<info_for_logging<TIME>> cpu_parallel_for_each_with_result_iterator(TIME t, ITERATOR first, ITERATOR last, FUNC& f, size_t thread_number = std::thread::hardware_concurrency()){
    		std::vector<info_for_logging<TIME>> logs;
    		/* get amount of elements to compute */
    		size_t n = std::distance(first, last);
    		/* if the amount of elements is less than the number of threads execute on n elements */
    	    if(n<thread_number){
    	    	thread_number=n;
    	    }
    	    /* OpenMP parallel loop */
    	    /* It doesn't work -> execution time as sequential version
    	    #pragma omp parallel for num_threads(thread_number) firstprivate(f, first) proc_bind(close) schedule(static)
    	    for(int i = 0; i < n; i++){
    	    	f(*(i+first));
    	    }
    	    */
    		#pragma omp parallel firstprivate(f) num_threads(thread_number) proc_bind(close)
    	    {
    	    	std::vector<info_for_logging<TIME>> partial_logs;
    	    	info_for_logging<TIME> result;
    	    	/* get thread id */
    	    	size_t tid = omp_get_thread_num();

    	    	/* if it's not last thread compute n/thread_number elements */
    	    	if(tid != thread_number-1) {
    	    		for(size_t i = (n/thread_number)*tid; i < (n/thread_number)*(tid+1); i++) {
    	    			result = f(*(i+first));
    	    			partial_logs.push_back(result);
    	    		}
    	    	/* if it's last thread compute till the end of the vector */
    			} else {
    				for(size_t i = (n/thread_number) * tid; i < n ; i++) {
    					result = f(*(i+first));
    					partial_logs.push_back(result);
    				}
    			}

				#pragma omp critical
    	    	{
    	    		logs.insert(logs.end(), std::make_move_iterator(partial_logs.begin()), std::make_move_iterator(partial_logs.end()));
    	    	}

    		}
    	    return logs;
    	}

    	template<typename TIME, typename T, typename FUNC>
    	std::vector<info_for_logging<TIME>> cpu_parallel_for_each_with_result(TIME t, std::vector<T>& obj, FUNC& f, size_t thread_number = std::thread::hardware_concurrency()){
    		std::vector<info_for_logging<TIME>> logs;
    		/* get amount of elements to compute */
    		size_t size = obj.size();

    		/* if the amount of elements is less than the number of threads execute on n elements */
    	    if(size<thread_number){
    	    	thread_number=size;
    	    }

    	    /* OpenMP parallel loop */
    	    /* It doesn't work -> execution time as sequential version
    	    #pragma omp parallel for num_threads(thread_number) firstprivate(f, first) proc_bind(close) schedule(static)
    	    for(int i = 0; i < n; i++){
    	    	f(*(i+first));
    	    }
    	    */

    		#pragma omp parallel firstprivate(f) num_threads(thread_number) proc_bind(close)
    	    {
    	    	// get thread id
    	    	size_t tid = omp_get_thread_num();
    	    	//set affinity
    	    	pin_thread_to_core(tid);

    	    	std::vector<info_for_logging<TIME>> partial_logs;
    	    	info_for_logging<TIME> result;

    	    	// determine first and last element to compute
    	    	size_t initial = (size/thread_number) * tid;

    	    	size_t last;
    	    	// if it's the last element compute till size, because division may not be perfect
    	    	if(tid != thread_number-1) {
    	    		last = (size/thread_number)*(tid+1);
    	    	} else {
    	    		last = size;
    	    	}

    	    	// compute size/thread_number elements
    	    	for(size_t i = initial; i < last; i++) {
    	    		result = f(*(obj.begin()+i));
    	    		partial_logs.push_back(result);
    	    	}

				#pragma omp critical
    	    	{
    	    		logs.insert(logs.end(), std::make_move_iterator(partial_logs.begin()), std::make_move_iterator(partial_logs.end()));
    	    	}


    		}
    	    return logs;
    	}



    	template<typename TIME, typename T, typename FUNC>
    	std::vector<info_for_logging<TIME>> gpu_parallel_for_each_with_result(TIME t, std::vector<T>& obj, FUNC& f, size_t thread_number = std::thread::hardware_concurrency()){

    		//info_for_logging<TIME> result;
    		std::vector<info_for_logging<TIME>> logs;
    	    /* get amount of elements to compute */
    	    //size_t n = std::distance(first, last);
    	    size_t size = obj.size();
    	    /* if the amount of elements is less than the number of threads execute on n elements */

			#pragma omp target teams distribute parallel for map(tofrom:obj) map(from:logs)
    	    for(size_t i = 0; i < size; i++){
    	    	//result = f(obj);
    	    	//logs.push_back(result);
    	    	logs.push_back(f(obj[i]));
    	    }

    	    return logs;
    	}

    	template<typename TIME, typename T, typename FUNC>
    	std::vector<info_for_logging<TIME>> multi_gpu_parallel_for_each_with_result(TIME t, std::vector<T>& obj, FUNC& f, size_t device_number = omp_get_num_devices()){
    		std::vector<info_for_logging<TIME>> logs;
    		/* get amount of elements to compute */
    		size_t size = obj.size();

    		/* if the amount of elements is less than the number of threads execute on n elements */
    	    if(size<device_number){
    	    	device_number=size;
    	    }

    	    T* p = obj.data();

    		#pragma omp parallel firstprivate(f) num_threads(device_number) proc_bind(close)
    	    {
    	    	std::vector<info_for_logging<TIME>> partial_logs;
    	    	info_for_logging<TIME> result;
    	    	// get thread id
    	    	size_t tid = omp_get_thread_num();

    	    	// determine first and last element to compute
    	    	size_t initial = (size/device_number) * tid;

    	    	size_t last;
    	    	// if it's the last element compute till size, because division may not be perfect
    	    	if(tid != device_number-1) {
    	    		last = (size/device_number)*(tid+1);
    	    	} else {
    	    		last = size;
    	    	}

    	    	/* compute size/devices_number elements */
    	    	#pragma omp target teams distribute parallel for map(tofrom:p[initial:last]) map(from:logs) device(tid)
    	    	for(size_t i = initial; i < last; i++) {
    	    		partial_logs.push_back(f(obj[i]));
    	    	}

				#pragma omp critical
    	    	{
    	    		logs.insert(logs.end(), std::make_move_iterator(partial_logs.begin()), std::make_move_iterator(partial_logs.end()));
    	    	}

    		}
    	    return logs;
    	}


    	template<typename TIME, typename T, typename FUNC>
    	std::vector<info_for_logging<TIME>> het_parallel_for_each_with_result(TIME t, std::vector<T>& obj, FUNC& f, size_t thread_number = std::thread::hardware_concurrency()){
    		std::vector<info_for_logging<TIME>> logs;
    		/* get amount of elements to compute */
    		size_t size = obj.size();

    		/* if the amount of elements is less than the number of threads execute on n elements */
    	    if(size<thread_number){
    	    	thread_number=size;
    	    }

    	    T* p = obj.data();

    		#pragma omp parallel firstprivate(f) num_threads(thread_number) proc_bind(close)
    	    {
    	    	std::vector<info_for_logging<TIME>> partial_logs;
    	    	info_for_logging<TIME> result;
    	    	// get thread id
    	    	size_t tid = omp_get_thread_num();

    	    	// determine first and last element to compute
    	    	size_t initial = (size/thread_number) * tid;

    	    	size_t last;
    	    	// if it's the last element compute till size, because division may not be perfect
    	    	if(tid != thread_number-1) {
    	    		last = (size/thread_number)*(tid+1);
    	    	} else {
    	    		last = size;
    	    	}

    	    	/* compute size/devices_number elements */
    	    	#pragma omp target teams distribute parallel for map(tofrom:p[initial:last]) map(from:logs) device(tid)
    	    	for(size_t i = initial; i < last; i++) {
    	    		partial_logs.push_back(f(obj[i]));
    	    	}

				#pragma omp critical
    	    	{
    	    		logs.insert(logs.end(), std::make_move_iterator(partial_logs.begin()), std::make_move_iterator(partial_logs.end()));
    	    	}

    		}
    	    return logs;
    	}



    	template<typename TIME, typename T, typename FUNC>
    	std::vector<info_for_logging<TIME>> hpc_parallel_for_each_with_result(TIME t, std::vector<T>& obj, FUNC& f, size_t thread_number = std::thread::hardware_concurrency()){
    		std::vector<info_for_logging<TIME>> logs;
    		/* get amount of elements to compute */
    		size_t size = obj.size();

    		/* if the amount of elements is less than the number of threads execute on n elements */
    	    if(size<thread_number){
    	    	thread_number=size;
    	    }

    	    /* OpenMP parallel loop */
    	    /* It doesn't work -> execution time as sequential version
    	    #pragma omp parallel for num_threads(thread_number) firstprivate(f, first) proc_bind(close) schedule(static)
    	    for(int i = 0; i < n; i++){
    	    	f(*(i+first));
    	    }
    	    */

    		#pragma omp parallel firstprivate(f) num_threads(thread_number) proc_bind(close)
    	    {
    	    	std::vector<info_for_logging<TIME>> partial_logs;
    	    	info_for_logging<TIME> result;
    	    	// get thread id
    	    	size_t tid = omp_get_thread_num();

    	    	// determine first and last element to compute
    	    	size_t initial = (size/thread_number) * tid;

    	    	size_t last;
    	    	// if it's the last element compute till size, because division may not be perfect
    	    	if(tid != thread_number-1) {
    	    		last = (size/thread_number)*(tid+1);
    	    	} else {
    	    		last = size;
    	    	}

    	    	// compute size/thread_number elements
    	    	for(size_t i = initial; i < last; i++) {
    	    		result = f(obj[i]);
    	    		partial_logs.push_back(result);
    	    	}

				#pragma omp critical
    	    	{
    	    		logs.insert(logs.end(), std::make_move_iterator(partial_logs.begin()), std::make_move_iterator(partial_logs.end()));
    	    	}


    		}
    	    return logs;
    	}




//    	template<typename LOGGER, typename TIME, typename ITERATOR, typename FUNC>
//    	void cpu_parallel_for_each_lambda_iterator(TIME t, ITERATOR first, ITERATOR last, FUNC& f, size_t thread_number = std::thread::hardware_concurrency()){

    		/* parallel lambda execution */
    		//std::vector<info_for_logging<TIME>> log = cpu_parallel_for_each_with_result(t, first, last, f, thread_number);

    		/* log results */
    		/*size_t n = log.size();
    		for(size_t i=0; i<n; i++){
    			if(log.at(i).type == "simulator"){
    				LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::sim_info_collect>(t, log.at(i).model_id);
    				if(log.at(i).imminent == true){
    					LOGGER::template log<cadmium::logger::logger_messages, cadmium::logger::sim_messages_collect>(t, log.at(i).model_id, log.at(i).messages_by_port);
    				}
    			}
    		}
    		*/
//    	}

//    	template<typename LOGGER, typename TIME, typename ITERATOR, typename FUNC>
//    	void cpu_parallel_for_each_delta_iterator(TIME t, ITERATOR first, ITERATOR last, FUNC& f, size_t thread_number = std::thread::hardware_concurrency()){

    		/* parallel delta execution */
    		//std::vector<info_for_logging<TIME>> log = cpu_parallel_for_each_with_result(t, first, last, f, thread_number);

    		/* log results */
    		/*size_t n = log.size();
    		for(size_t i=0; i<n; i++){
    			if(log.at(i).type == "simulator"){
    				LOGGER::template log<cadmium::logger::logger_info,cadmium::logger::sim_info_advance>(log.at(i).last, log.at(i).time, log.at(i).model_id);
    				LOGGER::template log<cadmium::logger::logger_local_time,cadmium::logger::sim_local_time>(log.at(i).last, log.at(i).time, log.at(i).model_id);
    				LOGGER::template log<cadmium::logger::logger_state,cadmium::logger::sim_state>(log.at(i).time, log.at(i).model_id, log.at(i).state_as_string);
    			}
    		}*/
//    	}


/*
    	template<typename LOGGER, typename TIME, typename T, typename FUNC>
    	void cpu_parallel_for_each_lambda(TIME t, std::vector<T>& obj, FUNC& f, size_t thread_number = std::thread::hardware_concurrency()){

    		// parallel lambda execution
    		std::vector<info_for_logging<TIME>> log = gpu_parallel_for_each_with_result(t, obj, f, thread_number);

    		// log results
    		size_t n = log.size();
    		for(size_t i=0; i<n; i++){
    			if(log.at(i).type == "simulator"){
    				LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::sim_info_collect>(t, log.at(i).model_id);
    				if(log.at(i).imminent == true){
    					LOGGER::template log<cadmium::logger::logger_messages, cadmium::logger::sim_messages_collect>(t, log.at(i).model_id, log.at(i).messages_by_port);
    				}
    			}
    		}
    	}

    	template<typename LOGGER, typename TIME, typename T, typename FUNC>
    	void cpu_parallel_for_each_delta(TIME t, std::vector<T>& obj, FUNC& f, size_t thread_number = std::thread::hardware_concurrency()){

    		// parallel delta execution
    		std::vector<info_for_logging<TIME>> log = gpu_parallel_for_each_with_result(t, obj, f, thread_number);

    		// log results
    		size_t n = log.size();
    		for(size_t i=0; i<n; i++){
    			if(log.at(i).type == "simulator"){
    				LOGGER::template log<cadmium::logger::logger_info,cadmium::logger::sim_info_advance>(log.at(i).last, log.at(i).time, log.at(i).model_id);
    				LOGGER::template log<cadmium::logger::logger_local_time,cadmium::logger::sim_local_time>(log.at(i).last, log.at(i).time, log.at(i).model_id);
    				LOGGER::template log<cadmium::logger::logger_state,cadmium::logger::sim_state>(log.at(i).time, log.at(i).model_id, log.at(i).state_as_string);
    			}
    		}
    	}
*/

    	template<typename LOGGER, typename TIME, typename T, typename FUNC>
    	void openmp_parallel_for_each_lambda(TIME t, std::vector<T>& obj, FUNC& f, size_t thread_number = std::thread::hardware_concurrency()){

    		std::vector<info_for_logging<TIME>> log;

    		/* parallel lambda execution */
			#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL
    			log = cpu_parallel_for_each_with_result(t, obj, f, thread_number);
			#else
				#if defined GPU_PARALLEL || defined GPU_LAMBDA_PARALLEL
    				log = gpu_parallel_for_each_with_result(t, obj, f, thread_number);
				#else
					#if defined MULTI_GPU_PARALLEL || defined MULTI_GPU_LAMBDA_PARALLEL
    					log = multi_gpu_parallel_for_each_with_result(t, obj, f, thread_number);
					#else
						#if defined HET_PARALLEL || defined HET_LAMBDA_PARALLEL
    						log = het_parallel_for_each_with_result(t, obj, f, thread_number);
						#else
							#if defined HPC
    							log = hpc_parallel_for_each_with_result(t, obj, f, thread_number);
							#endif
						#endif
					#endif
				#endif //GPU_PARALLEL
            #endif //CPU_PARALLEL

    		/* log results */
    		size_t n = log.size();
    		for(size_t i=0; i<n; i++){
    			if(log.at(i).type == "simulator"){
    				LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::sim_info_collect>(t, log.at(i).model_id);
    				if(log.at(i).imminent == true){
    					LOGGER::template log<cadmium::logger::logger_messages, cadmium::logger::sim_messages_collect>(t, log.at(i).model_id, log.at(i).messages_by_port);
    				}
    			}
    		}

    	}


    	template<typename LOGGER, typename TIME, typename T, typename FUNC>
    	void openmp_parallel_for_each_delta(TIME t, std::vector<T>& obj, FUNC& f, size_t thread_number = std::thread::hardware_concurrency()){

    		std::vector<info_for_logging<TIME>> log;

			#if defined CPU_PARALLEL || defined CPU_DELTA_PARALLEL
    		    log = cpu_parallel_for_each_with_result(t, obj, f, thread_number);
			#else
				#if defined GPU_PARALLEL || defined GPU_DELTA_PARALLEL
    		    	log = gpu_parallel_for_each_with_result(t, obj, f, thread_number);
				#else
					#if defined MULTI_GPU_PARALLEL || defined MULTI_GPU_DELTA_PARALLEL
    		    		log = multi_gpu_parallel_for_each_with_result(t, obj, f, thread_number);
					#else
						#if defined HET_PARALLEL || defined HET_DELTA_PARALLEL
    		    			log = het_parallel_for_each_with_result(t, obj, f, thread_number);
						#else
							#if defined HPC
    		    				log = hpc_parallel_for_each_with_result(t, obj, f, thread_number);
							#endif
						#endif
					#endif
				#endif //GPU_PARALLEL
			#endif //CPU_PARALLEL


    		/* log results */
    		size_t n = log.size();
    		for(size_t i=0; i<n; i++){
    			if(log.at(i).type == "simulator"){
    				LOGGER::template log<cadmium::logger::logger_info,cadmium::logger::sim_info_advance>(log.at(i).last, log.at(i).time, log.at(i).model_id);
    				LOGGER::template log<cadmium::logger::logger_local_time,cadmium::logger::sim_local_time>(log.at(i).last, log.at(i).time, log.at(i).model_id);
    				LOGGER::template log<cadmium::logger::logger_state,cadmium::logger::sim_state>(log.at(i).time, log.at(i).model_id, log.at(i).state_as_string);
    			}
    		}
    	}

    }
}

#endif //CADMIUM_PARALLEL_HELPERS_HPP
