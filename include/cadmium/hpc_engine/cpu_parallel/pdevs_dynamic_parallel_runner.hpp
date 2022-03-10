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

#ifndef CADMIUM_PDEVS_DYNAMIC_CPU_PARALLEL_RUNNER_HPP
#define CADMIUM_PDEVS_DYNAMIC_CPU_PARALLEL_RUNNER_HPP

#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/dynamic_common_loggers.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include <cadmium/hpc_engine/cpu_parallel/pdevs_dynamic_cpu_parallel_coordinator.hpp>
#include <cadmium/hpc_engine/cpu_parallel/pdevs_dynamic_cpu_parallel_engine_helpers.hpp>
#include <cadmium/hpc_engine/cpu_parallel/pdevs_dynamic_cpu_parallel_engine.hpp>
#include <omp.h>
#include <cadmium/hpc_engine/cpu_parallel/locks.hpp>
#include <cadmium/hpc_engine/parallel_helpers.hpp>

namespace cadmium {
    namespace dynamic {
        namespace hpc_engine {
            namespace cpu_parallel {
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
                class cpu_parallel_runner {
                    TIME _next; //next scheduled event

                    cadmium::dynamic::hpc_engine::cpu_parallel::cpu_parallel_coordinator<TIME, LOGGER> _top_coordinator; //this only works for coupled models.

                public:
                    //contructors
                    /**
                     * @brief set the dynamic parameters for the simulation
                     * @param init_time is the initial time of the simulation.
                     */
                    explicit cpu_parallel_runner(std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> coupled_model, const TIME &init_time)
                     : _top_coordinator(coupled_model){
                    }

                    TIME run_until(const TIME &t, const TIME &init_time, size_t _thread_number) {
                	    #if !defined(NO_LOGGER)
                	        LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(init_time);
                            LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Preparing model");
                            LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Starting run");
                        #endif

                        //create parallel region//
                        #pragma omp parallel num_threads(_thread_number) shared(_next, init_time, t, _top_coordinator)
                        {
                            //each thread gets its id//
                            size_t tid = omp_get_thread_num();

                            #pragma omp critical
                            {
                                cadmium:dynamic::hpc_engine::pin_thread_to_core(tid);
                            }

                            size_t n_subcoordinators =_top_coordinator.subcoordinators().size();
                            size_t n_internal_couplings =_top_coordinator.subcoordinators_internal_couplings().size();

                            size_t thread_number = omp_get_num_threads();
                            size_t n;

/*
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
*/
                            /************** PARALLEL STEP 0 - INITIALIZATION **************/

                            /************** PARALLEL STEP 0.1 - EXECUTE TIME ADVANCE **************/

                            //init partial subcoordinators
                            _top_coordinator.init_subcoordinators(first_subcoordinators, last_subcoordinators, init_time);



                            #pragma omp barrier

                            /************** END PARALLEL STEP 0.1 - COLLECT OUTPUTS **************/

                            /************** PARALLEL STEP 0.2 - EXECUTE TIME ADVANCE **************/

                            //calculate time for next event
                            _local_next = _top_coordinator.next_in_subcoordinators(first_subcoordinators,last_subcoordinators);

                            #pragma omp barrier

                            //only 1 threads initializes shared minimum wuth local minimum
                            #pragma omp single
	                        {
                    	        _next = _local_next;
                    	        _top_coordinator.set_next(_next);
    			            }
	                        //sync threads
	                        #pragma omp barrier

                            //1 thread at the time updates final result//
                            #pragma omp critical
                            {
                                if(_local_next < _next){
                        	        _next = _local_next;
                                    _top_coordinator.set_next(_next);
                                }
                            }

                            #pragma omp barrier

                            /************** END PARALLEL STEP 0.2 - EXECUTE TIME ADVANCE **************/

                            /************** PARALLEL STEP 0 - INITIALIZATION **************/

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
	               		        //collect partial outputs
	               		        _top_coordinator.collect_outputs_in_subcoordinators(first_subcoordinators, last_subcoordinators, _next);

                                #pragma omp barrier

	                            // ONLY 1 THREAD EXECUTES - collect outputs for external output couplings
	                            #pragma omp single
    	                        {
                                    // Use the EOC mapping to compose current level output
                                    _top_coordinator._outbox = cadmium::dynamic::hpc_engine::cpu_parallel::collect_messages_by_eoc<TIME, LOGGER>(_top_coordinator.external_output_couplings());
                                    //clean outbox because messages have been routed
                                    _top_coordinator._outbox = cadmium::dynamic::message_bags();
                                }

                                //sync threads
        			            #pragma omp barrier

                                #if !defined(NO_LOGGER)
                                    #pragma omp single
    	                            {
    	                                LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_advance>(_top_coordinator.last(), _next, _top_coordinator.get_model_id());
    	                            }
    	                            //sync threads
    	                            #pragma omp barrier
                                #endif

	                            /************** END PARALLEL STEP 1 - COLLECT OUTPUTS **************/

    	                        /************** PARALLEL STEP 2 - ROUTE MESSAGES **************/

                                _top_coordinator.route_internal_couplings_in_subcoordinators(first_internal_couplings, last_internal_couplings);

                                #pragma omp barrier

                                #pragma omp single
	                            {
                                    cadmium::dynamic::hpc_engine::cpu_parallel::route_external_input_coupled_messages_on_subcoordinators<TIME, LOGGER>(_top_coordinator.inbox(), _top_coordinator.external_input_couplings());
                                    //clean inbox because they were processed already
                                    _top_coordinator._inbox = cadmium::dynamic::message_bags();
                                }
                                #pragma omp barrier

	                            /************** END PARALLEL STEP 2 - ROUTE MESSAGES **************/

	                            /************** PARALLEL STEP 3 - STATE TRANSITION **************/

                                /* parallel state transition execution */
                                _top_coordinator.state_transition_in_subcoordinators(first_subcoordinators, last_subcoordinators, _next);

                                //sync threads
        			            #pragma omp barrier

        			    	    /************** END PARALLEL STEP 3 - STATE TRANSITION **************/

                                /************** PARALLEL STEP 4 - NEXT TIME **************/
                                _local_next = _top_coordinator.next_in_subcoordinators(first_subcoordinators,last_subcoordinators);

                                #pragma omp barrier

                                //only 1 threads initializes shared minimum wuth local minimum
                                #pragma omp single
    	                        {
        			    	        _next = _local_next;
        			    	        _top_coordinator.set_next(_next);
        			            }

                                #pragma omp barrier

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

                        #if !defined(NO_LOGGER)
                        LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Finished run");
                        #endif

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
