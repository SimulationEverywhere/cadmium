/**
 * Copyright (c) 2018, Damian Vicino, Laouen M. L. Belloli
 * Carleton University, Universite de Nice-Sophia Antipolis, Universidad de Buenos Aires
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

#ifndef CADMIUM_PDEVS_DYNAMIC_GPU_PARALLEL_RUNNER_HPP
#define CADMIUM_PDEVS_DYNAMIC_GPU_PARALLEL_RUNNER_HPP

#include <cadmium/parallel_engine/pdevs_dynamic_parallel_coordinator.hpp>

#include <cadmium/engine/pdevs_dynamic_engine.hpp>
#include <cadmium/engine/pdevs_dynamic_engine_helpers.hpp>
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/engine/pdevs_dynamic_simulator.hpp>
#include <cadmium/engine/pdevs_dynamic_engine.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/dynamic_common_loggers.hpp>
#include <cadmium/engine/pdevs_dynamic_engine_helpers.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include <cadmium/parallel_engine/parallel_helpers.hpp>
#include <omp.h>
#include <thread>


namespace cadmium {
    namespace dynamic {
        namespace parallel_engine {
            /**
             * @brief The Runner class runs the simulation.
             * The runner is in charge of setting up the coordinators and simulators, the initial
             * conditions, the ending conditions and the loggers, then it runs the simulation and
             * displays the results.
             *
             * @param Model The model to be simulated
             * @param Time Representation of time to be used to run the simualtion
             * @param Logger what, where and how to log from the simulation
             */

            //by default state changes get verbatim formatted and logged to cout
            template<typename TIME>
            using default_logger=cadmium::logger::logger<cadmium::logger::logger_state, cadmium::dynamic::logger::formatter<TIME>, cadmium::logger::cout_sink_provider>;

            //TODO: migrate specialization FEL behavior from CDBoost. At this point, there is no parametrized FEL.
            template<class TIME, typename LOGGER=default_logger<TIME>>
            class gpu_parallel_runner {
                TIME _next; //next scheduled event

                cadmium::dynamic::parallel_engine::parallel_coordinator<TIME, LOGGER> _top_coordinator; //this only works for coupled models.

            public:
                //contructors
                /**
                 * @brief set the dynamic parameters for the simulation
                 * @param init_time is the initial time of the simulation.
                 */


                explicit gpu_parallel_runner(std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> coupled_model, const TIME &init_time)
                 : _top_coordinator(coupled_model){

                    std::vector<cadmium::parallel::info_for_logging<TIME>> partial_logs;
                    // calculate start position/
                    size_t first_subcoordinators = 0;

                	// calculate end position/
                    size_t last_subcoordinators = _top_coordinator.subcoordinators().size();

                    #if !defined(NO_LOGGER)
             	        LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(init_time);
                        LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Preparing model");
                    #endif

                    //init partial subcoordinators
                    partial_logs = _top_coordinator.init_subcoordinators(first_subcoordinators, last_subcoordinators, init_time);

                    #if !defined(NO_LOGGER)
                        size_t n = partial_logs.size();
                        #pragma omp single
                        {
                     	    for(size_t i=0; i<n; i++){
                                LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::sim_info_init>(init_time, partial_logs.at(i).model_id);
                                LOGGER::template log<cadmium::logger::logger_state, cadmium::logger::sim_state>(init_time, partial_logs.at(i).model_id, partial_logs.at(i).state_as_string);
                     	    }
                        }
                    #endif

                    _next = _top_coordinator.next_in_subcoordinators(first_subcoordinators, last_subcoordinators);
                    _top_coordinator.set_next(_next);
                }

                explicit gpu_parallel_runner(std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> coupled_model, const TIME &init_time, size_t _thread_number)
                    : _top_coordinator(coupled_model){

                    #if !defined(NO_LOGGER)
                	    LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(init_time);
                        LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Preparing model");
                    #endif

                    //create parallel region//
                    #pragma omp parallel num_threads(_thread_number) shared(_next)
                	{
                        //each thread gets its id//
                        size_t tid = omp_get_thread_num();

                        #pragma omp critical
                        {
                            cadmium:parallel::pin_thread_to_core(tid);
                        }

                        std::vector<cadmium::parallel::info_for_logging<TIME>> partial_logs;
                        cadmium::parallel::info_for_logging<TIME> log_info;

                        size_t n_subcoordinators =_top_coordinator.subcoordinators().size();

                        size_t thread_number = omp_get_num_threads();
                        size_t n;

                        TIME _local_next = _next;

                        //calculate number of elements to compute//
                        size_t local_n_subcoordinators = n_subcoordinators/thread_number;

                        // calculate start position/
                        size_t first_subcoordinators = tid*local_n_subcoordinators;

                        // calculate end position/
                        size_t last_subcoordinators;

                        if(tid != (thread_number-1)){
                            last_subcoordinators = (tid+1)*local_n_subcoordinators;
                        } else {
                            last_subcoordinators = _top_coordinator.subcoordinators().size();
                        }

                        //init partial subcoordinators
                        partial_logs = _top_coordinator.init_subcoordinators(first_subcoordinators, last_subcoordinators, init_time);

                        #pragma omp barrier

						#if !defined(NO_LOGGER)
                            n = partial_logs.size();
                            #pragma omp single
                            {
                        	    for(size_t i=0; i<n; i++){
                                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::sim_info_init>(init_time, partial_logs.at(i).model_id);
                                    LOGGER::template log<cadmium::logger::logger_state, cadmium::logger::sim_state>(init_time, partial_logs.at(i).model_id, partial_logs.at(i).state_as_string);
                        	    }
                            }
                            #pragma omp barrier
                       #endif

                        //_next = _top_coordinator.next();

                        _local_next = _top_coordinator.next_in_subcoordinators(first_subcoordinators,last_subcoordinators);

                        //only 1 threads initializes shared minimum wuth local minimum
                        #pragma omp single
	                    {
                    	    _next = _local_next;
                    	    _top_coordinator.set_next(_next);
    			        }

                        //1 thread at the time updates final result//
                        #pragma omp critical
                        {
                            if(_local_next < _next){
                        	    _next = _local_next;
                                _top_coordinator.set_next(_next);
                            }
                        }

                        #pragma omp barrier
                	}

                }
                
            TIME sequential_run_until(const TIME &t) {
                std::vector<cadmium::parallel::info_for_logging<TIME>> log_info;
                std::vector<cadmium::dynamic::logger::routed_messages> log_routed;

                #if !defined(NO_LOGGER)
                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Starting run");
                #endif

                cadmium:parallel::pin_thread_to_core(0);

                std::vector<cadmium::parallel::info_for_logging<TIME>> partial_logs;
                //cadmium::parallel::info_for_logging<TIME> log_info;

                std::vector<cadmium::dynamic::logger::routed_messages> partial_logs_routed;
                std::vector<cadmium::dynamic::logger::routed_messages> result_routed;

                size_t n;

                TIME _local_next = _next;

                // calculate start position//
                size_t first_subcoordinators = 0;
                size_t first_internal_couplings = 0;

                // calculate end position/
                size_t last_subcoordinators, last_internal_couplings;

                last_subcoordinators = _top_coordinator.subcoordinators().size();
                last_internal_couplings = _top_coordinator.subcoordinators_internal_couplings().size();

                // simulation cycle loop
                while (_next < t) {

            	    if(_top_coordinator.next() < _next){
            	        throw std::domain_error("Trying to obtain output when not internal event is scheduled");
            	    }

            	    //INITIAL LOG (IF NO_LOGGER IS NOT DEFINED)
            	    #if !defined(NO_LOGGER)
            	        LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(_top_coordinator.next());
                        LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_collect>(_next, _top_coordinator.get_model_id());
                        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eoc_collect>(_next, _top_coordinator.get_model_id());
                    #endif

            	    /************** PARALLEL STEP 1 - COLLECT OUTPUTS **************/

                    //collect partial outputs
                    partial_logs = _top_coordinator.collect_outputs_in_subcoordinators(first_subcoordinators, last_subcoordinators, _next);

            	    //SEQUENTIAL PART - COLLECT OUTPUT LOG (IF NO_LOGGER IS NOT DEFINED)
            	    #if !defined(NO_LOGGER)
                        n = partial_logs.size();
            	        //1 THREAD AT THE TIME//
                        for(size_t i=0; i<n; i++){
                            LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::sim_info_collect>(_next, partial_logs.at(i).model_id);
                            if(partial_logs.at(i).imminent == true){
                                LOGGER::template log<cadmium::logger::logger_messages, cadmium::logger::sim_messages_collect>(_next, partial_logs.at(i).model_id, partial_logs.at(i).messages_by_port);
                            }
                        }
            	        //clear partial logs
            	        partial_logs.clear();
                    #endif

            	    // Use the EOC mapping to compose current level output
                    _top_coordinator._outbox = cadmium::dynamic::parallel_engine::collect_messages_by_eoc<TIME, LOGGER>(_top_coordinator.external_output_couplings());
                    //clean outbox because messages have been routed
                    _top_coordinator._outbox = cadmium::dynamic::message_bags();

                    #if !defined(NO_LOGGER)
                        LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_advance>(_top_coordinator.last(), _next, _top_coordinator.get_model_id());
                    #endif

            	    /************** END PARALLEL STEP 1 - COLLECT OUTPUTS **************/

                	/************** PARALLEL STEP 2 - ROUTE MESSAGES **************/

                    partial_logs_routed = _top_coordinator.internal_couplings_routing(first_internal_couplings, last_internal_couplings);

                    //ROUTE MESSAGE LOG (IF NO_LOGGER IS NOT DEFINED)
                    #if !defined(NO_LOGGER)
                        n = partial_logs_routed.size();
                        //1 THREAD AT THE TIME//
                        for(size_t i=0; i<n; i++){
                            LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_collect>(partial_logs_routed.at(i).from_port, partial_logs_routed.at(i).to_port, partial_logs_routed.at(i).from_messages, partial_logs_routed.at(i).to_messages);
                        }
                        //clear partial logs
                        partial_logs_routed.clear();
                        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eic_collect>(_next, _top_coordinator.get_model_id());
                    #endif

                    cadmium::dynamic::parallel_engine::route_external_input_coupled_messages_on_subcoordinators<TIME, LOGGER>(_top_coordinator.inbox(), _top_coordinator.external_input_couplings());
                    //clean inbox because they were processed already
                    _top_coordinator._inbox = cadmium::dynamic::message_bags();

            	    /************** END PARALLEL STEP 2 - ROUTE MESSAGES **************/

            	    /************** PARALLEL STEP 3 - STATE TRANSITION **************/

                    /* parallel delta execution */
                    partial_logs = _top_coordinator.state_transition_in_subcoordinators(first_subcoordinators, last_subcoordinators, _next);

                    //SEQUENTIAL PART - STATE TRANSITION LOG (IF NO_LOGGER IS NOT DEFINED)
                    #if !defined(NO_LOGGER)
                        n = partial_logs.size();
                        //1 THREAD AT THE TIME//
                        for(size_t i=0; i<n; i++){
                            LOGGER::template log<cadmium::logger::logger_info,cadmium::logger::sim_info_advance>(partial_logs.at(i).last, partial_logs.at(i).time, partial_logs.at(i).model_id);
                            LOGGER::template log<cadmium::logger::logger_local_time,cadmium::logger::sim_local_time>(partial_logs.at(i).last, partial_logs.at(i).time, partial_logs.at(i).model_id);
                            LOGGER::template log<cadmium::logger::logger_state,cadmium::logger::sim_state>(partial_logs.at(i).time, partial_logs.at(i).model_id, partial_logs.at(i).state_as_string);
                        }
                        //clear partial logs
                        partial_logs.clear();
                    #endif

                    /************** END PARALLEL STEP 3 - STATE TRANSITION **************/

                    /************** PARALLEL STEP 4 - NEXT TIME **************/

                    _next = _top_coordinator.next_in_subcoordinators(first_subcoordinators,last_subcoordinators);
                    _top_coordinator.set_next(_next);

                    /************** END PARALLEL STEP 4 - NEXT TIME **************/

            	}//end simulation loop

                LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Finished run");
                return _next;
            }

            TIME sequential_run_until_no_log(const TIME &t) {

                std::vector<cadmium::dynamic::logger::routed_messages> partial_logs_routed;
                std::vector<cadmium::dynamic::logger::routed_messages> result_routed;

                size_t n;

                TIME _local_next = _next;

                // calculate start position//
                size_t first_subcoordinators = 0;
                size_t first_internal_couplings = 0;

                // calculate end position/
                size_t last_subcoordinators, last_internal_couplings;

                last_subcoordinators = _top_coordinator.subcoordinators().size();
                last_internal_couplings = _top_coordinator.subcoordinators_internal_couplings().size();

                // simulation cycle loop
                while (_next < t) {

            	    if(_top_coordinator.next() < _next){
            	        throw std::domain_error("Trying to obtain output when not internal event is scheduled");
            	    }

            	    /************** PARALLEL STEP 1 - COLLECT OUTPUTS **************/

                    //collect partial outputs
                    _top_coordinator.collect_outputs_in_subcoordinators_no_log(first_subcoordinators, last_subcoordinators, _next);

            	    // Use the EOC mapping to compose current level output
                    _top_coordinator._outbox = cadmium::dynamic::parallel_engine::collect_messages_by_eoc<TIME, LOGGER>(_top_coordinator.external_output_couplings());
                    //clean outbox because messages have been routed
                    _top_coordinator._outbox = cadmium::dynamic::message_bags();

            	    /************** END PARALLEL STEP 1 - COLLECT OUTPUTS **************/

                	/************** PARALLEL STEP 2 - ROUTE MESSAGES **************/

                    partial_logs_routed = _top_coordinator.internal_couplings_routing(first_internal_couplings, last_internal_couplings);

                    cadmium::dynamic::parallel_engine::route_external_input_coupled_messages_on_subcoordinators<TIME, LOGGER>(_top_coordinator.inbox(), _top_coordinator.external_input_couplings());
                    //clean inbox because they were processed already
                    _top_coordinator._inbox = cadmium::dynamic::message_bags();

            	    /************** END PARALLEL STEP 2 - ROUTE MESSAGES **************/

            	    /************** PARALLEL STEP 3 - STATE TRANSITION **************/

                    /* parallel delta execution */
                    _top_coordinator.state_transition_in_subcoordinators(first_subcoordinators, last_subcoordinators, _next);

                    /************** END PARALLEL STEP 3 - STATE TRANSITION **************/

                    /************** PARALLEL STEP 4 - NEXT TIME **************/

                    _next = _top_coordinator.next_in_subcoordinators(first_subcoordinators,last_subcoordinators);
                    _top_coordinator.set_next(_next);

                    /************** END PARALLEL STEP 4 - NEXT TIME **************/

            	}//end simulation loop

                return _next;
            }





            TIME parallel_run_until(const TIME &t, size_t _thread_number) {
                std::vector<cadmium::parallel::info_for_logging<TIME>> log_info;
                std::vector<cadmium::dynamic::logger::routed_messages> log_routed;

                #if !defined(NO_LOGGER)
                std::cout << "NO_LOGGER" << std::endl;
                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Starting run");
                #endif

                //create parallel region//
                #pragma omp parallel num_threads(_thread_number) shared(_next, _top_coordinator)
                {
                    //each thread gets its id//
                	size_t tid = omp_get_thread_num();

					#pragma omp critical
                    {
                        cadmium:parallel::pin_thread_to_core(tid);
                    }

                        std::vector<cadmium::parallel::info_for_logging<TIME>> partial_logs;
                        cadmium::parallel::info_for_logging<TIME> log_info;

                        std::vector<cadmium::dynamic::logger::routed_messages> partial_logs_routed;
                        std::vector<cadmium::dynamic::logger::routed_messages> result_routed;

                        size_t n_subcoordinators =_top_coordinator.subcoordinators().size();
                        size_t n_internal_couplings =_top_coordinator.subcoordinators_internal_couplings().size();

                        size_t thread_number = omp_get_num_threads();
                        size_t n;

                        TIME _local_next = _next;

                        //calculate number of elements to compute//
                        size_t local_n_subcoordinators = n_subcoordinators/thread_number;
                        size_t local_n_internal_couplings = n_internal_couplings/thread_number;

                        // calculate start position/
                        size_t first_subcoordinators = tid*local_n_subcoordinators;
                        size_t first_internal_couplings = tid*local_n_internal_couplings;

                        // calculate end position/
                        size_t last_subcoordinators, last_internal_couplings;

                        if(tid != (thread_number-1)){
                            last_subcoordinators = (tid+1)*local_n_subcoordinators;
                            last_internal_couplings = (tid+1)*local_n_internal_couplings;
                        } else {
                            last_subcoordinators = _top_coordinator.subcoordinators().size();
                            last_internal_couplings = _top_coordinator.subcoordinators_internal_couplings().size();
                        }

                        // simulation cycle loop
                        while (_next < t) {

                            #pragma omp single
	               		    {
	               		        if(_top_coordinator.next() < _next){
	               		            throw std::domain_error("Trying to obtain output when not internal event is scheduled");
	               		        }
	               		    }

                		    //SEQUENTIAL PART - INITIAL LOG (IF NO_LOGGER IS NOT DEFINED)
                            #if !defined(NO_LOGGER)
					            #pragma omp single
	               		        {
                			        LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(_top_coordinator.next());
                			        LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_collect>(_next, _top_coordinator.get_model_id());
                			        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eoc_collect>(_next, _top_coordinator.get_model_id());
                			    }
	               		        //sync threads
	               		        #pragma omp barrier
	               		    #endif

	               		    /************** PARALLEL STEP 1 - COLLECT OUTPUTS **************/
                            /*
	               				for(size_t i = first; i < last; i++){
	               					log_info = _top_coordinator._subcoordinators.at(i)->collec_outputs_without_logging(t);
	               					partial_logs.push_back(log_info);
	               				}
                            */
	               		    //collect partial outputs
	               		    partial_logs = _top_coordinator.collect_outputs_in_subcoordinators(first_subcoordinators, last_subcoordinators, _next);

	                        //SEQUENTIAL PART - COLLECT OUTPUT LOG (IF NO_LOGGER IS NOT DEFINED)
	                        #if !defined(NO_LOGGER)
	               			    n = partial_logs.size();
	                            //1 THREAD AT THE TIME//
	                            #pragma omp critical
	               				{
            			            for(size_t i=0; i<n; i++){
            			                LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::sim_info_collect>(_next, partial_logs.at(i).model_id);
            			                if(partial_logs.at(i).imminent == true){
            			                    LOGGER::template log<cadmium::logger::logger_messages, cadmium::logger::sim_messages_collect>(_next, partial_logs.at(i).model_id, partial_logs.at(i).messages_by_port);
            			                }
            			            }
            			        }
	                            //clear partial logs
	                            partial_logs.clear();
                            #endif

	                        // ONLY 1 THREAD EXECUTES - collect outputs for external output couplings
	                        #pragma omp single
    	                    {
                                // Use the EOC mapping to compose current level output
                                _top_coordinator._outbox = cadmium::dynamic::parallel_engine::collect_messages_by_eoc<TIME, LOGGER>(_top_coordinator.external_output_couplings());
                                //clean outbox because messages have been routed
                                _top_coordinator._outbox = cadmium::dynamic::message_bags();
                            }

                            //sync threads
        			        #pragma omp barrier

                            #if !defined(NO_LOGGER)
    	                        LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_advance>(_top_coordinator.last(), _next, _top_coordinator.get_model_id());
    	                        //sync threads
    	                        #pragma omp barrier
                            #endif

	                        /************** END PARALLEL STEP 1 - COLLECT OUTPUTS **************/

    	                    /************** PARALLEL STEP 2 - ROUTE MESSAGES **************/

                            partial_logs_routed = _top_coordinator.internal_couplings_routing(first_internal_couplings, last_internal_couplings);

                            //SEQUENTIAL PART - ROUTE MESSAGE LOG (IF NO_LOGGER IS NOT DEFINED)
                            #if !defined(NO_LOGGER)
                                n = partial_logs_routed.size();
                                //1 THREAD AT THE TIME//
                                #pragma omp critical
                                {
                                    for(size_t i=0; i<n; i++){
                                        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_collect>(partial_logs_routed.at(i).from_port, partial_logs_routed.at(i).to_port, partial_logs_routed.at(i).from_messages, partial_logs_routed.at(i).to_messages);
                                    }
                                }
                                //clear partial logs
                                partial_logs_routed.clear();

                                #pragma omp barrier

                                #pragma omp single
                                {
                                    LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eic_collect>(_next, _top_coordinator.get_model_id());
                                }

                            #endif

                            #pragma omp single
	                        {
                                cadmium::dynamic::parallel_engine::route_external_input_coupled_messages_on_subcoordinators<TIME, LOGGER>(_top_coordinator.inbox(), _top_coordinator.external_input_couplings());
                                //clean inbox because they were processed already
                                _top_coordinator._inbox = cadmium::dynamic::message_bags();
                            }
                            #pragma omp barrier

	                        /************** END PARALLEL STEP 2 - ROUTE MESSAGES **************/

	                        /************** PARALLEL STEP 3 - STATE TRANSITION **************/

                            /* parallel delta execution */
                            partial_logs = _top_coordinator.state_transition_in_subcoordinators(first_subcoordinators, last_subcoordinators, _next);

                            //SEQUENTIAL PART - STATE TRANSITION LOG (IF NO_LOGGER IS NOT DEFINED)
                            #if !defined(NO_LOGGER)
                                n = partial_logs.size();
                                //1 THREAD AT THE TIME//
                                #pragma omp critical
                                {
                                    for(size_t i=0; i<n; i++){
                                        LOGGER::template log<cadmium::logger::logger_info,cadmium::logger::sim_info_advance>(partial_logs.at(i).last, partial_logs.at(i).time, partial_logs.at(i).model_id);
                                	    LOGGER::template log<cadmium::logger::logger_local_time,cadmium::logger::sim_local_time>(partial_logs.at(i).last, partial_logs.at(i).time, partial_logs.at(i).model_id);
                                	    LOGGER::template log<cadmium::logger::logger_state,cadmium::logger::sim_state>(partial_logs.at(i).time, partial_logs.at(i).model_id, partial_logs.at(i).state_as_string);
                                    }
                                }
                                //clear partial logs
                                partial_logs.clear();
                            #endif

                            //sync threads
        			        #pragma omp barrier

        			    	/************** END PARALLEL STEP 3 - STATE TRANSITION **************/

                            /************** PARALLEL STEP 4 - NEXT TIME **************/
                            _local_next = _top_coordinator.next_in_subcoordinators(first_subcoordinators,last_subcoordinators);
/*
        			    	    _local_next = _top_coordinator->get_next();

        			    	    for(size_t i = first; i < last; i++){
        			    	    	if(partial_logs.at(i).imminent_or_receiver == true) {
        			    	    		if(_top_coordinator.subcoordinators().at(i)->next() < _local_next){
        			    	    			_local_next = _top_coordinator.subcoordinators().at(i)->next();
        			    	    		}
        			    	    	}
        			    	    }
*/
                            //only 1 threads initializes shared minimum wuth local minimum
                            #pragma omp single
    	                    {
        			    	    _next = _local_next;
        			    	    _top_coordinator.set_next(_next);
        			        }

                            // calculate final result from partial_results sequentially /
                            #pragma omp critical
                            {
                                if(_local_next < _next){
                            	    _next = _local_next;
                            	    _top_coordinator.set_next(_next);
                                }
                            }

                            #pragma omp barrier
    	                   /************** END PARALLEL STEP 4 - NEXT TIME **************/

	                    }//end simulation loop

                    }//end parallel region

                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Finished run");
                    return _next;
                }

                /**
                 * @brief runUntilPassivate starts the simulation and stops when there is no next internal event to happen.
                 */
                void run_until_passivate() {
                    run_until(std::numeric_limits<TIME>::infinity());
                }


            };
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_RUNNER_HPP
