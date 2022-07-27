/**
 * Copyright (c) 2022, Guillermo Trabes
 * Carleton University, Universidad Nacional de San Luis
 *
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

#ifndef CADMIUM_PDEVS_DYNAMIC_PARALLEL_RUNNER_HPP
#define CADMIUM_PDEVS_DYNAMIC_PARALLEL_RUNNER_HPP

#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/dynamic_common_loggers.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include <cadmium/hpc_engine/parallel/pdevs_dynamic_parallel_coordinator.hpp>
#include <cadmium/hpc_engine/parallel/pdevs_dynamic_parallel_engine_helpers.hpp>
#include <cadmium/hpc_engine/parallel/pdevs_dynamic_parallel_simulator.hpp>
#include <cadmium/hpc_engine/parallel/locks.hpp>
#include <cadmium/hpc_engine/parallel_helpers.hpp>
#include <omp.h>

namespace cadmium {
    namespace dynamic {
        namespace hpc_engine {
            namespace parallel {
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
                class parallel_runner {
                    TIME _next; //next scheduled event
                    cadmium::dynamic::hpc_engine::parallel::parallel_coordinator<TIME, LOGGER> _top_coordinator; //this only works for coupled models.

                public:
                    //contructors
                    /**
                     * @brief set the dynamic parameters for the simulation
                     * @param init_time is the initial time of the simulation.
                     */
                    explicit parallel_runner(std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> coupled_model)
                    : _top_coordinator(coupled_model) {
                    }

                    /**
                     * @brief runUntil starts the simulation and stops when the next event is scheduled after t.
                     * @param t is the limit time for the simulation.
                     * @return the TIME of the next event to happen when simulation stopped.
                     */
                    TIME run_until(const TIME &init_time, const TIME &t, int _thread_number) {

                        #if !defined(NO_LOGGER)
                        LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(init_time);
                        LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Preparing model");
                        #endif

                        //create parallel region//
                        #pragma omp parallel num_threads(_thread_number) shared(_next, init_time, t, _top_coordinator) //proc_bind(close)
                        {

                            subcoordinators_type<TIME, LOGGER> subcoordinators = _top_coordinator.subcoordinators();
                            internal_couplings<TIME, LOGGER> internal_couplings = _top_coordinator.get_internal_couplings();

                            size_t n_subcoordinators =_top_coordinator.subcoordinators().size();
                            size_t n_internal_couplings =_top_coordinator.get_internal_couplings().size();

                            TIME local_next;

                            //each thread gets its id//
                            int tid = omp_get_thread_num();

                        	#pragma omp critical
                        	{
                        	    cadmium:dynamic::hpc_engine::pin_thread_to_core(tid);
                        	}

                        	 /************** PARALLEL STEP 0.1 - EXECUTE TIME ADVANCE **************/
/*
                            #pragma omp master
                        	{
                                _top_coordinator.init(init_time);
                         	}
*/

                            #if !defined(NO_LOGGER)
                                #pragma omp master
	               		        {
                                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_init>(init_time, _top_coordinator.get_model_id());
	               		        }
                            #endif

                            #if defined DYNAMiC
                              #pragma omp for schedule(dynamic)
                            #else
                              #pragma omp for schedule(static)
                            #endif
                            for(size_t i=0; i<n_subcoordinators;i++){
                                subcoordinators[i]->init(init_time);
                            }

                            #pragma omp barrier

                            /************** END PARALLEL STEP 0.1 - COLLECT OUTPUTS **************/

                            /************** PARALLEL STEP 0.2 - EXECUTE TIME ADVANCE **************/
/*
                            #pragma omp master
                        	{
                                _next = _top_coordinator.next_in_subcoordinators();
                        	}
*/

                        	local_next = subcoordinators[0]->next();

                        	//#pragma omp for schedule(schedule) nowait
                            #if defined DYNAMIC
                              #pragma omp for schedule(dynamic) nowait
                            #else
                              #pragma omp for schedule(static) nowait
                            #endif
                        	for(size_t i=1; i<n_subcoordinators;i++){
                        	    if(subcoordinators[i]->next()<local_next){
                        	        local_next = subcoordinators[i]->next();
                        	    }
                        	}

                            #pragma omp single
                            {
                                 _next = local_next;
                            }

                            #pragma omp barrier

                            #pragma omp critical
		                    {
			                    if(local_next < _next){
				                    _next = local_next;
			                    }
		                    }

                            #pragma omp barrier

		                    /************** END PARALLEL STEP 0.2 - EXECUTE TIME ADVANCE **************/

                            #if !defined(NO_LOGGER)
                                #pragma omp single
                        	    {
                                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Starting run");
                        	    }
                            #endif

                            while (_next < t) {

                            	/************** PARALLEL STEP 1 - COLLECT OUTPUTS **************/

                            	#if !defined(NO_LOGGER)
                                    #pragma omp master
                            	    {
                                        LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(_next);
                                        LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_collect>(_next, _top_coordinator.get_model_id());
                                        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eoc_collect>(_next, _top_coordinator.get_model_id());
                                    }
                                    #pragma omp barrier
                                #endif

/*
                                #pragma omp master
                        	    {
                                _top_coordinator.collect_outputs(_next);
                        	    }
*/

                                //#pragma omp for schedule(schedule)
                                #if defined DYNAMIC
                                  #pragma omp for schedule(dynamic)
                                #else
                                  #pragma omp for schedule(static)
                                #endif
                                for(size_t i=0; i<n_subcoordinators;i++){
                                    subcoordinators[i]->collect_outputs(_next);
                        	    }

                                /************** END PARALLEL STEP 1 - COLLECT OUTPUTS **************/

                                /************** PARALLEL STEP 2 - ROUTE MESSAGES **************/
/*
                                if !defined(NO_LOGGER)
                                    #pragma omp single
                        	        {
                                        LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_advance>(_top_coordinator.last(), _next, _top_coordinator.get_model_id());
                                        //Route the messages standing in the outboxes to mapped inboxes following ICs and EICs
                                        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_ic_collect>(_next, _top_coordinator.get_model_id());
                                    }
                                #endif

                                #pragma omp barrier
*/
/*
                                #pragma omp master
                        	    {
                        	    _top_coordinator.route_messages(_next);
                        	    }
*/

                        	    //Route internal couplings
                                //#pragma omp for schedule(schedule) //collapse(2)
                                #if defined DYNAMIC
                                  #pragma omp for schedule(dynamic)
                                #else
                                  #pragma omp for schedule(static)
                                #endif
                                for(size_t i=0; i<n_internal_couplings; i++){
                                    for(size_t j=0; j<internal_couplings[i].second.size(); j++){
                                        auto& from_outbox = internal_couplings[i].first.first->outbox();
                                        auto& to_inbox = internal_couplings[i].first.second->inbox();

                                        cadmium::dynamic::hpc_engine::parallel::set_route_lock(typeid(to_inbox));
                                        cadmium::dynamic::logger::routed_messages message_to_log = internal_couplings[i].second[j]->route_messages(from_outbox, to_inbox);
                                        cadmium::dynamic::hpc_engine::parallel::release_route_lock(typeid(to_inbox));

                                        #if !defined(NO_LOGGER)
                                        cadmium::dynamic::hpc_engine::parallel::set_log_lock();
                                        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_collect>(message_to_log.from_port, message_to_log.to_port, message_to_log.from_messages, message_to_log.to_messages);
                                        cadmium::dynamic::hpc_engine::parallel::release_log_lock();
                                        #endif
                                    }
                                }

//                                #pragma omp barrier

                                /************** END PARALLEL STEP 2 - ROUTE MESSAGES **************/

                                /************** PARALLEL STEP 3 - STATE TRANSITION **************/
/*
                                #pragma omp master
                        	    {
                                _top_coordinator.state_transition(_next);
                        	    }
*/

                                //#pragma omp for schedule(schedule)
                                #if defined DYNAMIC
                                  #pragma omp for schedule(dynamic)
                                #else
                                  #pragma omp for schedule(static)
                                #endif
                        	    for(size_t i=0; i<n_subcoordinators;i++){
                        	    	subcoordinators[i]->state_transition(_next);
                        	    }

//                                #pragma omp barrier
                        	    /************** END PARALLEL STEP 3 - STATE TRANSITION **************/

                        	    /************** PARALLEL STEP 4 - NEXT TIME **************/
/*
                                #pragma omp master
                        	    {
                                _next = _top_coordinator.next_in_subcoordinators();
                        	    }
*/

                                local_next = subcoordinators[0]->next();

//                                #pragma omp for schedule(schedule) nowait
                                #if defined DYNAMIC
                                  #pragma omp for schedule(dynamic)
                                #else
                                  #pragma omp for schedule(static)
                                #endif
                        	    for(size_t i=1; i<n_subcoordinators;i++){
                        	        if(subcoordinators[i]->next()<local_next){
                        	            local_next = subcoordinators[i]->next();
                        	        }
                        	    }

                                #pragma omp single
                                {
                                    _next = local_next;
                                }

                                #pragma omp barrier

                                #pragma omp critical
		                        {
			                        if(local_next < _next){
				                        _next = local_next;
			                        }
		                        }

                                #pragma omp barrier

		                        /************** END PARALLEL STEP 4 - NEXT TIME **************/

                            }// end loop

                            #if !defined(NO_LOGGER)
                                #pragma omp master
                                {
                                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Finished run");
                                }
                            #endif

                        }
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
}

#endif //CADMIUM_PDEVS_DYNAMIC_RUNNER_HPP
