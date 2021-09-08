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

#ifndef CADMIUM_PDEVS_DYNAMIC_RUNNER_HPP
#define CADMIUM_PDEVS_DYNAMIC_RUNNER_HPP

#include <cadmium/engine/pdevs_dynamic_coordinator.hpp>

#ifdef CADMIUM_EXECUTE_CONCURRENT
#include <boost/thread/executors/basic_thread_pool.hpp>
#endif //CADMIUM_EXECUTE_CONCURRENT

#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
#include <thread>
#include <cadmium/engine/parallel_helpers.hpp>
#include <omp.h>
#endif //CPU_PARALLEL

#include <cadmium/engine/pdevs_dynamic_engine.hpp>
#include <cadmium/engine/pdevs_dynamic_engine_helpers.hpp>


#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/engine/pdevs_dynamic_simulator.hpp>
#include <cadmium/engine/pdevs_dynamic_engine.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/dynamic_common_loggers.hpp>
#include <cadmium/engine/pdevs_dynamic_engine_helpers.hpp>
#include <cadmium/logger/common_loggers.hpp>

namespace cadmium {
    namespace dynamic {
        namespace engine {
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
            class runner {
                TIME _next; //next scheduled event

                cadmium::dynamic::engine::coordinator<TIME, LOGGER> _top_coordinator; //this only works for coupled models.

                #ifdef CADMIUM_EXECUTE_CONCURRENT
                boost::basic_thread_pool _threadpool;
                #endif //CADMIUM_EXECUTE_CONCURRENT

				#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                size_t _thread_number;
				#endif//CPU_PARALLEL

            public:
                //contructors
                /**
                 * @brief set the dynamic parameters for the simulation
                 * @param init_time is the initial time of the simulation.
                 */

				#ifdef CADMIUM_EXECUTE_CONCURRENT
                explicit runner(std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> coupled_model, const TIME &init_time, unsigned const thread_count = boost::thread::hardware_concurrency())
                : _top_coordinator(coupled_model),
                  _threadpool(thread_count){
                    LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(init_time);
                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Preparing model");
                    _top_coordinator.init(init_time, &_threadpool);
                    _next = _top_coordinator.next();
                }
                #else
					#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                	explicit runner(std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> coupled_model, const TIME &init_time, unsigned const thread_number = std::thread::hardware_concurrency())
                    	: _top_coordinator(coupled_model){
                		_thread_number = thread_number;
                		LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(init_time);
                		LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Preparing model");
                		_top_coordinator.init(init_time, _thread_number);
                		_next = _top_coordinator.next();
                    	}
					#else
                	explicit runner(std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> coupled_model, const TIME &init_time)
                	: _top_coordinator(coupled_model){
                		LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(init_time);
                		LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Preparing model");
                		_top_coordinator.init(init_time);
                		_next = _top_coordinator.next();
                	}
					#endif //CPU_PARALLEL
                #endif //CADMIUM_EXECUTE_CONCURRENT
                

                /**
                 * @brief runUntil starts the simulation and stops when the next event is scheduled after t.
                 * @param t is the limit time for the simulation.
                 * @return the TIME of the next event to happen when simulation stopped.
                 */

				#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                	TIME run_until2(const TIME &t) {

                		std::vector<cadmium::parallel::info_for_logging<TIME>> log_info;
                		std::vector<cadmium::dynamic::logger::routed_messages> log_routed;

                		LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Starting run");

						#pragma omp parallel num_threads(_thread_number) shared(_next)
                		{
                			size_t tid = omp_get_thread_num();

							#pragma omp critical
                			{
                				cadmium:parallel::pin_thread_to_core(tid);
                			}

							std::vector<cadmium::parallel::info_for_logging<TIME>> partial_logs;
							cadmium::parallel::info_for_logging<TIME> result;

							std::vector<cadmium::dynamic::logger::routed_messages> partial_logs_routed;
							std::vector<cadmium::dynamic::logger::routed_messages> result_routed;

							size_t n_subcoordinators =_top_coordinator.subcoordinators().size();
							size_t n_internal_couplings =_top_coordinator.subcoordinators_internal_couplings().size();

	               			size_t thread_number = omp_get_num_threads();
	               			size_t n;

	               			//size_t primero, ultimo;

	               			//typename subcoordinators_type<TIME>::iterator first_subcoordinators, last_subcoordinators;
	               			//typename internal_couplings<TIME>::iterator first_internal_couplings, last_internal_couplings;

	               			//subcoordinators_type<TIME> _subcoordinators;
	               			//external_couplings<TIME> _external_output_couplings;
	               			//external_couplings<TIME> _external_input_couplings;
	               			//internal_couplings<TIME> _internal_coupligns;

	               			//std::vector<std::shared_ptr<cadmium::dynamic::engine::engine<TIME>>>::iterator first_subcoordinators, last_subcoordinators;
	               			//std::vector<internal_coupling<TIME>>::iterator first_internal_coupligns, last_internal_couplings;

	               			//auto first_subcoordinators, last_subcoordinators;
	               			//auto first_internal_coupligns, last_internal_couplings;

							TIME _local_next = _next;

	               			// calculate number of elements to compute /
	               			size_t local_n_subcoordinators = n_subcoordinators/thread_number;
	               			size_t local_n_internal_couplings = n_internal_couplings/thread_number;
	               			// calculate start position/
	               			//first_subcoordinators = _top_coordinator.subcoordinators().begin()+(tid*local_n_subcoordinators);
	               			//first_internal_couplings = _top_coordinator.subcoordinators_internal_couplings().begin()+(tid*local_n_internal_couplings);
	               			size_t first = tid*local_n_subcoordinators;
							size_t first_internal_couplings = tid*local_n_internal_couplings;
	               			// calculate end position/
	               			//if(tid != (thread_number-1)){
	               			//auto	last_subcoordinators = first_subcoordinators+local_n_subcoordinators;
	               			//auto last_internal_couplings = first_internal_couplings+local_n_internal_couplings;
	               			//}else{
	               			//	last_subcoordinators = _top_coordinator.subcoordinators_internal_couplings().end();
	               			//	last_internal_couplings = _top_coordinator.subcoordinators_internal_couplings().end();
	               			//}

							//size_t last_internal_couplings;
	               			size_t last, last_internal_couplings;


							if(tid != (thread_number-1)){
								//last_subcoordinators = (tid+1)*local_n_subcoordinators;
								//last_internal_couplings = (tid+1)*local_n_internal_couplings;
								//last_subcoordinators = _top_coordinator.subcoordinators().begin()+((tid+1)*local_n_subcoordinators);
								//last_internal_couplings = _top_coordinator.subcoordinators_internal_couplings().begin()+((tid+1)*local_n_internal_couplings);
								last = (tid+1)*local_n_subcoordinators;
								last_internal_couplings = (tid+1)*local_n_internal_couplings;
							} else {
								last = _top_coordinator.subcoordinators().size();
								last_internal_couplings = _top_coordinator.subcoordinators_internal_couplings().size();
								//last_internal_couplings = n_internal_couplings;
							}


	               			//auto last_subcoordinators = _top_coordinator.subcoordinators().begin()+(tid+1*local_n_subcoordinators);
	               			//auto last_internal_couplings = _top_coordinator.subcoordinators_internal_couplings().begin()+(tid+1*local_n_internal_couplings);

							//_next = _top_coordinator.next();

							//#pragma omp barrier

                			// simulation cycle loop
	               			while (_next < t) {

                				//SEQUENTIAL PART
                				//if(tid == 0) {
								#pragma omp single
	               				{
                					LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(_top_coordinator.next());
                					LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_collect>(_next, _top_coordinator.get_model_id());
                					//if (_top_coordinator.next() < _next || _next < top_coordinator.last() ) {
                					if (_top_coordinator.next() < _next) {
                						//throw std::domain_error("Trying to obtain output when out of the advance time scope");
                						//throw std::domain_error("Trying to obtain output when out of the advance time scope");
                						throw std::domain_error("Trying to obtain output when not internal event is scheduled");
                					}
                					//log EOC
                					LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eoc_collect>(_next, _top_coordinator.get_model_id());
                					// Fill all outboxes and clean the inboxes in the lower levels recursively
                				}

                				//sync threads
                				#pragma omp barrier

                				//PARALLEL PART
                					partial_logs = _top_coordinator.collect_outputs_in_subcoordinators(first, last, _next);
                					n = partial_logs.size();
/*
                					#pragma omp critical
                					{
                						log_info.insert(log_info.end(), std::make_move_iterator(partial_logs.begin()), std::make_move_iterator(partial_logs.end()));
                					}
*/
                				//sync threads
//                				#pragma omp barrier

								#pragma omp critical
	               				{
            			    		// log results //
            			    		for(size_t i=0; i<n; i++){
            			    			if(partial_logs.at(i).type == "simulator"){
            			    				LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::sim_info_collect>(_next, partial_logs.at(i).model_id);
            			    				if(partial_logs.at(i).imminent == true){
            			    					LOGGER::template log<cadmium::logger::logger_messages, cadmium::logger::sim_messages_collect>(_next, partial_logs.at(i).model_id, partial_logs.at(i).messages_by_port);
            			    				}
            			    			}
            			    		}
            			    		//partial_logs.clear();
            			    	}


                					//clear partial logs
                					partial_logs.clear();

                					//sync threads
        			    	    	#pragma omp barrier

        			    	    //SEQUENTIAL PART
            			    	//if(tid == 0){
/*
								#pragma omp single
	               				{
            			    		// log results //
            			    		n = log_info.size();
            			    		for(size_t i=0; i<n; i++){
            			    			if(log_info.at(i).type == "simulator"){
            			    				LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::sim_info_collect>(_next, log_info.at(i).model_id);
            			    				if(log_info.at(i).imminent == true){
            			    					LOGGER::template log<cadmium::logger::logger_messages, cadmium::logger::sim_messages_collect>(_next, log_info.at(i).model_id, log_info.at(i).messages_by_port);
            			    				}
            			    			}
            			    		}
            			    		log_info.clear();
            			    	}
*/

                				//if(tid == 0){
                				#pragma omp single
    	               			{
                					// Use the EOC mapping to compose current level output
                					_top_coordinator._outbox = cadmium::dynamic::engine::collect_messages_by_eoc<TIME, LOGGER>(_top_coordinator.external_output_couplings());
                					//clean outbox because messages are routed before calling this function at a higher level
                					_top_coordinator._outbox = cadmium::dynamic::message_bags();
                					LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_advance>(_top_coordinator.last(), _next, _top_coordinator.get_model_id());
                				}

                                //sync threads
                                #pragma omp barrier

                                //result_routed = _top_coordinator._subcoordinators.at(i).route_messages();
                				partial_logs_routed = _top_coordinator.internal_couplings_routing(first_internal_couplings, last_internal_couplings);
                				n = partial_logs_routed.size();
                	    	    //partial_logs.push_back(result);
/*
                        		#pragma omp critical
                        	    {
                        	    	log_routed.insert(log_routed.end(), std::make_move_iterator(partial_logs_routed.begin()), std::make_move_iterator(partial_logs_routed.end()));
                        	    }
*/
                				//sync threads
//                				#pragma omp barrier

								#pragma omp critical
								{
                					// log results /
                            		for(size_t i=0; i<n; i++){
                            			LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_collect>(partial_logs_routed.at(i).from_port, partial_logs_routed.at(i).to_port, partial_logs_routed.at(i).from_messages, partial_logs_routed.at(i).to_messages);
                            		}
                            		//clear routing log info
                            		//log_routed.clear();
	               				}

                        	    partial_logs_routed.clear();

								#pragma omp barrier

/*
                                //if(tid == 0){
								#pragma omp single
	               				{
                                	// log results /
                                	n = log_routed.size();
                                	for(size_t i=0; i<n; i++){
                                		LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_collect>(log_routed.at(i).from_port, log_routed.at(i).to_port, log_routed.at(i).from_messages, log_routed.at(i).to_messages);
                                	}
                                	//clear routing log info
                                	log_routed.clear();
                                }
*/

/*
								#pragma omp critical
        			    	    {
        			    	    	std::cout << "after routing messages" << std::endl;
        			    	    }
*/

								//#pragma omp barrier

                                //if (tid == 0){
								#pragma omp single
	               				{
                                	LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eic_collect>(_next, _top_coordinator.get_model_id());
                                	cadmium::dynamic::engine::route_external_input_coupled_messages_on_subcoordinators<TIME, LOGGER>(_top_coordinator.inbox(), _top_coordinator.external_input_couplings());
                                }

								#pragma omp barrier

                                /* parallel delta execution */
                                partial_logs = _top_coordinator.apply_delta_without_logging(first, last, _next);
                                //partial_logs = _top_coordinator.apply_delta_without_logging(first_subcoordinators, last_subcoordinators, _next);
                                n = partial_logs.size();
            					//partial_logs.push_back(result);
                                //}
/*
            					#pragma omp critical
                                {
                                	log_info.insert(log_info.end(), std::make_move_iterator(partial_logs.begin()), std::make_move_iterator(partial_logs.end()));
                                }
*/

//								#pragma omp barrier

                                #pragma omp critical
    	               			{
                            		// log results /
                            		for(size_t i=0; i<n; i++){
                            			if(partial_logs.at(i).type == "simulator"){
                            				LOGGER::template log<cadmium::logger::logger_info,cadmium::logger::sim_info_advance>(partial_logs.at(i).last, partial_logs.at(i).time, partial_logs.at(i).model_id);
                            				LOGGER::template log<cadmium::logger::logger_local_time,cadmium::logger::sim_local_time>(partial_logs.at(i).last, partial_logs.at(i).time, partial_logs.at(i).model_id);
                            				LOGGER::template log<cadmium::logger::logger_state,cadmium::logger::sim_state>(partial_logs.at(i).time, partial_logs.at(i).model_id, partial_logs.at(i).state_as_string);
                            			}
                            		}
                            		//log_info.clear();
                            	}


        			    	    partial_logs.clear();

/*
								#pragma omp critical
        			    	    {
        			    	    	//std::cout << "after apply transition" << std::endl;
        			    	    }
*/

        			    	    //sync threads

        			    	    #pragma omp barrier

/*
                            	//if(tid == 0){
                            	#pragma omp single
    	               			{
                            		// log results /

                					n = log_info.size();
                            		for(size_t i=0; i<n; i++){
                            			if(log_info.at(i).type == "simulator"){
                            				LOGGER::template log<cadmium::logger::logger_info,cadmium::logger::sim_info_advance>(log_info.at(i).last, log_info.at(i).time, log_info.at(i).model_id);
                            				LOGGER::template log<cadmium::logger::logger_local_time,cadmium::logger::sim_local_time>(log_info.at(i).last, log_info.at(i).time, log_info.at(i).model_id);
                            				LOGGER::template log<cadmium::logger::logger_state,cadmium::logger::sim_state>(log_info.at(i).time, log_info.at(i).model_id, log_info.at(i).state_as_string);
                            			}
                            		}
                            		log_info.clear();
                            	}
*/
                				// end collect outputs in top coordinator
								//#pragma omp barrier

                				//_next = _top_coordinator.next();
                            	//_local_next = _top_coordinator.next_in_subcoordinators(first_subcoordinators, last_subcoordinators);

                            	//size_t it = last-first;
                                /*
                            	_local_next = _top_coordinator.subcoordinators().at(first_subcoordinators)->next();

                            	for(size_t i = first_subcoordinators; i < last_subcoordinators; i++){
                            		if(_top_coordinator.subcoordinators().at(i)->next() < _local_next){
                            			_local_next = _top_coordinator.subcoordinators().at(i)->next();
                            		}
                            	}
                            	*/

//								#pragma omp critical
//        			    	    {
        			    	    	//std::cout <<"tid: " << tid << " time: "<< _next << " local next: " << _local_next << std::endl;
//        			    	    }

								#pragma omp critical
        			    	    {
        			    	    	_local_next = _top_coordinator.next_in_subcoordinators(first,last);
        			    	    }

//								#pragma omp barrier

//								#pragma omp critical
//        			    	    {
        			    	    	//std::cout <<"tid: " << tid << " time: "<< _next << " local next: " << _local_next << std::endl;
//        			    	    }

        			    	    //if(tid == 0){
        			    	    #pragma omp single
								//#pragma omp critical
    	               			{
        			    	    	_next = _local_next;
        			    	    }

        			    	    //#pragma omp barrier

                            	// calculate final result from partial_results sequentially /
                            	#pragma omp critical
                            	{
                            		if(_local_next < _next){
                            			_next = _local_next;
                            		}
                            	}

//								#pragma omp barrier

//								#pragma omp critical
//        			    	    {
        			    	    	//std::cout <<"tid: " << tid << " time: "<< _next << " local next: " << _local_next << std::endl;
//        			    	    }

//								#pragma omp barrier


/*
								#pragma omp single
        			    	    {
                            	//if (tid == 0){
                            		_next = _top_coordinator.min_next_in_subcoordinators(_top_coordinator.subcoordinators(),1);
                            	}
*/


/*
								#pragma omp critical
        			    	    {
                            		//std::cout << "after calculte next time step" << std::endl;
        			    	    }
*/
								#pragma omp barrier
                            	//sequential
                            	//if(tid == 0){
                            	#pragma omp single
    	               			{
                            		_top_coordinator.set_next(_next);
                            		//_next
                            		//clean inbox because they were processed already
                            		_top_coordinator._inbox = cadmium::dynamic::message_bags();
                            	}

								#pragma omp barrier

	               			}//end simulation loop

                		}//end parallel region

                		LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Finished run");
                		return _next;
                	}
				#else
                	TIME run_until2(const TIME &t) {
                		LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Starting run");
                		while (_next < t) {
                			LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(_next);
                			_top_coordinator.collect_outputs(_next);
                			_top_coordinator.advance_simulation(_next);
                			_next = _top_coordinator.next();
                		}
                		LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Finished run");
                		return _next;
                	}
				#endif


				#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                	TIME run_until(const TIME &t) {

                		std::vector<cadmium::parallel::info_for_logging<TIME>> log_info;
                		std::vector<cadmium::dynamic::logger::routed_messages> log_routed;

                		LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Starting run");

						#pragma omp parallel num_threads(_thread_number) shared(_next)
                		{
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

	               			// calculate number of elements to compute /
	               			size_t local_n_subcoordinators = n_subcoordinators/thread_number;
	               			size_t local_n_internal_couplings = n_internal_couplings/thread_number;
	               			// calculate start position/
	               			//first_subcoordinators = _top_coordinator.subcoordinators().begin()+(tid*local_n_subcoordinators);
	               			//first_internal_couplings = _top_coordinator.subcoordinators_internal_couplings().begin()+(tid*local_n_internal_couplings);
	               			size_t first = tid*local_n_subcoordinators;
							size_t first_internal_couplings = tid*local_n_internal_couplings;
	               			// calculate end position/
	               			//if(tid != (thread_number-1)){
	               			//auto	last_subcoordinators = first_subcoordinators+local_n_subcoordinators;
	               			//auto last_internal_couplings = first_internal_couplings+local_n_internal_couplings;
	               			//}else{
	               			//	last_subcoordinators = _top_coordinator.subcoordinators_internal_couplings().end();
	               			//	last_internal_couplings = _top_coordinator.subcoordinators_internal_couplings().end();
	               			//}

							//size_t last_internal_couplings;
	               			size_t last, last_internal_couplings;


							if(tid != (thread_number-1)){
								last = (tid+1)*local_n_subcoordinators;
								last_internal_couplings = (tid+1)*local_n_internal_couplings;
							} else {
								last = _top_coordinator.subcoordinators().size();
								last_internal_couplings = _top_coordinator.subcoordinators_internal_couplings().size();
								//last_internal_couplings = n_internal_couplings;
							}

                			// simulation cycle loop
	               			while (_next < t) {

                				//SEQUENTIAL PART
								#pragma omp single
	               				{
                					LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(_top_coordinator.next());
                					LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_collect>(_next, _top_coordinator.get_model_id());

                					if (_top_coordinator.next() < _next) {
                						throw std::domain_error("Trying to obtain output when not internal event is scheduled");
                					}
                					//log EOC
                					LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eoc_collect>(_next, _top_coordinator.get_model_id());
                					// Fill all outboxes and clean the inboxes in the lower levels recursively
                				}

                				//sync threads
                				#pragma omp barrier

                				//PARALLEL PART
	               				/************ STEP 1 ***********/
/*
	               				for(size_t i = first; i < last; i++){
	               					log_info = _top_coordinator._subcoordinators.at(i)->collec_outputs_without_logging(t);
	               					partial_logs.push_back(log_info);
	               				}
*/
	               				//PARALLEL PARt
	               				partial_logs = _top_coordinator.collect_outputs_in_subcoordinators(first, last, _next);

	               				n = partial_logs.size();

								#pragma omp critical
	               				{
            			    		// log results //
            			    		for(size_t i=0; i<n; i++){
            			    			if(partial_logs.at(i).type == "simulator"){
            			    				LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::sim_info_collect>(_next, partial_logs.at(i).model_id);
            			    				if(partial_logs.at(i).imminent == true){
            			    					LOGGER::template log<cadmium::logger::logger_messages, cadmium::logger::sim_messages_collect>(_next, partial_logs.at(i).model_id, partial_logs.at(i).messages_by_port);
            			    				}
            			    			}
            			    		}
            			    	}

                				//clear partial logs
                				partial_logs.clear();

                				//sync threads
        			    	    #pragma omp barrier

                				/************ END STEP 1 ***********/

                				//if(tid == 0){
                				#pragma omp single
    	               			{
                					// Use the EOC mapping to compose current level output
                					_top_coordinator._outbox = cadmium::dynamic::engine::collect_messages_by_eoc<TIME, LOGGER>(_top_coordinator.external_output_couplings());
                					//clean outbox because messages are routed before calling this function at a higher level
                					_top_coordinator._outbox = cadmium::dynamic::message_bags();
                					LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_advance>(_top_coordinator.last(), _next, _top_coordinator.get_model_id());
                				}

                                //sync threads
                                #pragma omp barrier

                                //result_routed = _top_coordinator._subcoordinators.at(i).route_messages();
                				partial_logs_routed = _top_coordinator.internal_couplings_routing(first_internal_couplings, last_internal_couplings);
                				n = partial_logs_routed.size();
                	    	    //partial_logs.push_back(result);

                				/*
                				for(size_t i = first_internal_couplings; i < last_internal_couplings; i++){
                					log_info _top_coordinator.subcoordinators.at(i)->collec_outputs_without_logging(t);
                					partial_logs.push_back(log_info);
                				}
                				*/
/*
                        		#pragma omp critical
                        	    {
                        	    	log_routed.insert(log_routed.end(), std::make_move_iterator(partial_logs_routed.begin()), std::make_move_iterator(partial_logs_routed.end()));
                        	    }
*/
                				//sync threads
//                				#pragma omp barrier

								#pragma omp critical
								{
                					// log results /
                            		for(size_t i=0; i<n; i++){
                            			LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_collect>(partial_logs_routed.at(i).from_port, partial_logs_routed.at(i).to_port, partial_logs_routed.at(i).from_messages, partial_logs_routed.at(i).to_messages);
                            		}
                            		//clear routing log info
                            		//log_routed.clear();
	               				}

                        	    partial_logs_routed.clear();

								#pragma omp barrier

/*
                                //if(tid == 0){
								#pragma omp single
	               				{
                                	// log results /
                                	n = log_routed.size();
                                	for(size_t i=0; i<n; i++){
                                		LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_collect>(log_routed.at(i).from_port, log_routed.at(i).to_port, log_routed.at(i).from_messages, log_routed.at(i).to_messages);
                                	}
                                	//clear routing log info
                                	log_routed.clear();
                                }
*/

/*
								#pragma omp critical
        			    	    {
        			    	    	std::cout << "after routing messages" << std::endl;
        			    	    }
*/

								//#pragma omp barrier

                                //if (tid == 0){
								#pragma omp single
	               				{
                                	LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eic_collect>(_next, _top_coordinator.get_model_id());
                                	cadmium::dynamic::engine::route_external_input_coupled_messages_on_subcoordinators<TIME, LOGGER>(_top_coordinator.inbox(), _top_coordinator.external_input_couplings());
                                }

								#pragma omp barrier

                                /* parallel delta execution */
                                partial_logs = _top_coordinator.apply_delta_without_logging(first, last, _next);
                                //partial_logs = _top_coordinator.apply_delta_without_logging(first_subcoordinators, last_subcoordinators, _next);
                                n = partial_logs.size();
            					//partial_logs.push_back(result);
                                //}
/*
            					#pragma omp critical
                                {
                                	log_info.insert(log_info.end(), std::make_move_iterator(partial_logs.begin()), std::make_move_iterator(partial_logs.end()));
                                }
*/

//								#pragma omp barrier

                                #pragma omp critical
    	               			{
                            		// log results /
                            		for(size_t i=0; i<n; i++){
                            			if(partial_logs.at(i).type == "simulator"){
                            				LOGGER::template log<cadmium::logger::logger_info,cadmium::logger::sim_info_advance>(partial_logs.at(i).last, partial_logs.at(i).time, partial_logs.at(i).model_id);
                            				LOGGER::template log<cadmium::logger::logger_local_time,cadmium::logger::sim_local_time>(partial_logs.at(i).last, partial_logs.at(i).time, partial_logs.at(i).model_id);
                            				LOGGER::template log<cadmium::logger::logger_state,cadmium::logger::sim_state>(partial_logs.at(i).time, partial_logs.at(i).model_id, partial_logs.at(i).state_as_string);
                            			}
                            		}
                            		//log_info.clear();
                            	}


        			    	    partial_logs.clear();

/*
								#pragma omp critical
        			    	    {
        			    	    	//std::cout << "after apply transition" << std::endl;
        			    	    }
*/

        			    	    //sync threads

        			    	    #pragma omp barrier

/*
                            	//if(tid == 0){
                            	#pragma omp single
    	               			{
                            		// log results /

                					n = log_info.size();
                            		for(size_t i=0; i<n; i++){
                            			if(log_info.at(i).type == "simulator"){
                            				LOGGER::template log<cadmium::logger::logger_info,cadmium::logger::sim_info_advance>(log_info.at(i).last, log_info.at(i).time, log_info.at(i).model_id);
                            				LOGGER::template log<cadmium::logger::logger_local_time,cadmium::logger::sim_local_time>(log_info.at(i).last, log_info.at(i).time, log_info.at(i).model_id);
                            				LOGGER::template log<cadmium::logger::logger_state,cadmium::logger::sim_state>(log_info.at(i).time, log_info.at(i).model_id, log_info.at(i).state_as_string);
                            			}
                            		}
                            		log_info.clear();
                            	}
*/
                				// end collect outputs in top coordinator
								//#pragma omp barrier

                				//_next = _top_coordinator.next();
                            	//_local_next = _top_coordinator.next_in_subcoordinators(first_subcoordinators, last_subcoordinators);

                            	//size_t it = last-first;
                                /*
                            	_local_next = _top_coordinator.subcoordinators().at(first_subcoordinators)->next();

                            	for(size_t i = first_subcoordinators; i < last_subcoordinators; i++){
                            		if(_top_coordinator.subcoordinators().at(i)->next() < _local_next){
                            			_local_next = _top_coordinator.subcoordinators().at(i)->next();
                            		}
                            	}
                            	*/

//								#pragma omp critical
//        			    	    {
        			    	    	//std::cout <<"tid: " << tid << " time: "<< _next << " local next: " << _local_next << std::endl;
//        			    	    }

								//#pragma omp critical
        			    	    //{
        			    	    	_local_next = _top_coordinator.next_in_subcoordinators(first,last);
        			    	    //}
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
//								#pragma omp barrier

//								#pragma omp critical
//        			    	    {
        			    	    	//std::cout <<"tid: " << tid << " time: "<< _next << " local next: " << _local_next << std::endl;
//        			    	    }

        			    	    //if(tid == 0){
        			    	    #pragma omp single
								//#pragma omp critical
    	               			{
        			    	    	_next = _local_next;
        			    	    }

        			    	    //#pragma omp barrier

                            	// calculate final result from partial_results sequentially /
                            	#pragma omp critical
                            	{
                            		if(_local_next < _next){
                            			_next = _local_next;
                            		}
                            	}

//								#pragma omp barrier

//								#pragma omp critical
//        			    	    {
        			    	    	//std::cout <<"tid: " << tid << " time: "<< _next << " local next: " << _local_next << std::endl;
//        			    	    }

//								#pragma omp barrier


/*
								#pragma omp single
        			    	    {
                            	//if (tid == 0){
                            		_next = _top_coordinator.min_next_in_subcoordinators(_top_coordinator.subcoordinators(),1);
                            	}
*/


/*
								#pragma omp critical
        			    	    {
                            		//std::cout << "after calculte next time step" << std::endl;
        			    	    }
*/
								#pragma omp barrier
                            	//sequential
                            	//if(tid == 0){
                            	#pragma omp single
    	               			{
                            		_top_coordinator.set_next(_next);
                            		//_next
                            		//clean inbox because they were processed already
                            		_top_coordinator._inbox = cadmium::dynamic::message_bags();
                            	}

    	               			partial_logs.clear();

								#pragma omp barrier

	               			}//end simulation loop

                		}//end parallel region

                		LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Finished run");
                		return _next;
                	}
				#else
                	TIME run_until(const TIME &t) {
                		LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Starting run");
                		while (_next < t) {
                			LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(_next);
                			_top_coordinator.collect_outputs(_next);
                			_top_coordinator.advance_simulation(_next);
                			_next = _top_coordinator.next();
                		}
                		LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Finished run");
                		return _next;
                	}
				#endif











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
