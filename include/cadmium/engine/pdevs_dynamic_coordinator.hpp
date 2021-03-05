/**
 * Copyright (c) 2018, Laouen M. L. Belloli, Damian Vicino
 * Carleton University, Universidad de Buenos Aires, Universite de Nice-Sophia Antipolis
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

#ifndef CADMIUM_PDEVS_DYNAMIC_COORDINATOR_HPP
#define CADMIUM_PDEVS_DYNAMIC_COORDINATOR_HPP

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

            template<typename TIME, typename LOGGER>
            class coordinator : public cadmium::dynamic::engine::engine<TIME> {

                //MODEL is assumed valid, the whole model tree is checked at "runner level" to fail fast
                TIME _last; //last transition time
                TIME _next; // next transition scheduled

                std::string _model_id;

                subcoordinators_type<TIME> _subcoordinators;
                external_couplings<TIME> _external_output_couplings;
                external_couplings<TIME> _external_input_couplings;
                internal_couplings<TIME> _internal_coupligns;

                #ifdef CADMIUM_EXECUTE_CONCURRENT
                boost::basic_thread_pool* _threadpool;
                #endif //CADMIUM_EXECUTE_CONCURRENT

				#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                size_t _thread_number;
				#endif //CPU_PARALLEL

            public:

                dynamic::message_bags _inbox;
                dynamic::message_bags _outbox;

                using model_type = typename cadmium::dynamic::modeling::coupled<TIME>;

                /**
                 * @brief A dynamic coordinator must be constructed with the coupled model to coordinate.
                 */
                coordinator() = delete;

                coordinator(std::shared_ptr<model_type> coupled_model)
                        : _model_id(coupled_model->get_id())
                {
                    #ifdef CADMIUM_EXECUTE_CONCURRENT
                    _threadpool = nullptr;
                    #endif //CADMIUM_EXECUTE_CONCURRENT

                    std::map<std::string, std::shared_ptr<engine<TIME>>> engines_by_id;

                    for(auto& m : coupled_model->_models) {
                        std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::modeling::coupled<TIME>>(m);
                        std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> m_atomic = std::dynamic_pointer_cast<cadmium::dynamic::modeling::atomic_abstract<TIME>>(m);

                        if (m_coupled == nullptr) {
                            if (m_atomic == nullptr) {
                                throw std::domain_error("Invalid submodel is neither coupled nor atomic");
                            }
                            std::shared_ptr<cadmium::dynamic::engine::engine<TIME>> simulator = std::make_shared<cadmium::dynamic::engine::simulator<TIME, LOGGER>>(m_atomic);
                            _subcoordinators.push_back(simulator);
                        } else {
                            if (m_atomic != nullptr) {
                                throw std::domain_error("Invalid submodel is defined as both coupled and atomic");
                            }
                            std::shared_ptr<cadmium::dynamic::engine::engine<TIME>> coordinator = std::make_shared<cadmium::dynamic::engine::coordinator<TIME, LOGGER>>(m_coupled);
                            _subcoordinators.push_back(coordinator);
                        }

                        engines_by_id.insert(std::make_pair(_subcoordinators.back()->get_model_id(), _subcoordinators.back()));
                    }

                    // Generates structures for direct access to external couplings to not iterate all coordinators each time.

                    for (const auto& eoc : coupled_model->_eoc) {
                    	if (engines_by_id.find(eoc._from) == engines_by_id.end()) {
                    		throw std::domain_error("External output coupling from invalid model");
                    	}

                    	cadmium::dynamic::engine::external_coupling<TIME> new_eoc;
                    	new_eoc.first = engines_by_id.at(eoc._from);
                    	new_eoc.second.push_back(eoc._link);
                    	_external_output_couplings.push_back(new_eoc);
                    }

                    for (const auto& eic : coupled_model->_eic) {
                    	if (engines_by_id.find(eic._to) == engines_by_id.end()) {
                    		throw std::domain_error("External input coupling to invalid model");
                    	}

                    	cadmium::dynamic::engine::external_coupling<TIME> new_eic;
                    	new_eic.first = engines_by_id.at(eic._to);
                    	new_eic.second.push_back(eic._link);
                    	_external_input_couplings.push_back(new_eic);

                    }

                    for (const auto& ic : coupled_model->_ic) {
                    	if (engines_by_id.find(ic._from) == engines_by_id.end() || engines_by_id.find(ic._to) == engines_by_id.end()) {
                    		throw std::domain_error("Internal coupling to invalid model");
                    	}

                    	cadmium::dynamic::engine::internal_coupling<TIME> new_ic;
                    	new_ic.first.first = engines_by_id.at(ic._from);
                    	new_ic.first.second = engines_by_id.at(ic._to);
                    	new_ic.second.push_back(ic._link);
                    	_internal_coupligns.push_back(new_ic);
                    }

                }
                /**
                 * @brief init function sets the start time
                 * @param initial_time is the start time
                 */
                void init(TIME initial_time) override {
                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_init>(initial_time, _model_id);

                    _last = initial_time;
                    //init all subcoordinators and find next transition time.

					#ifdef CADMIUM_EXECUTE_CONCURRENT
                    cadmium::dynamic::engine::init_subcoordinators<TIME>(initial_time, _subcoordinators, _threadpool);
                    #else
						#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                    	cadmium::dynamic::engine::init_subcoordinators<TIME>(initial_time, _subcoordinators, _thread_number);
						#else
                    	cadmium::dynamic::engine::init_subcoordinators<TIME>(initial_time, _subcoordinators);
						#endif
                    #endif //CADMIUM_EXECUTE_CONCURRENT

                    //find the one with the lowest next time
                    //_next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);
					#if defined CPU_PARALLEL || defined CPU_MIN_PARALLEL
                    	_next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators, _thread_number);
                        //_next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);
					#else
                    	_next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);
                    #endif

                }

                #ifdef CADMIUM_EXECUTE_CONCURRENT
                void init(TIME initial_time, boost::basic_thread_pool* threadpool) override {
                    _threadpool = threadpool;
                    this->init(initial_time);
                }
                #endif //CADMIUM_EXECUTE_CONCURRENT

				#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                void init(TIME initial_time, size_t thread_number) {
                    _thread_number = thread_number;
                    this->init(initial_time);
                }
                #endif //CPU_PARALLEL


                std::string get_model_id() const override {
                    return _model_id;
                }

                /**
                 * @brief Coordinator expected next internal transition time
                 */
                TIME next() const noexcept override {
                    return _next;
                }

                TIME last() const noexcept {
                	return _last;
                }

                void set_next(TIME t) {
                	_next = t;
                }


                //subcoordinators_type<TIME> _subcoordinators;
                //external_couplings<TIME> _external_output_couplings;
                //external_couplings<TIME> _external_input_couplings;
                //internal_couplings<TIME> _internal_coupligns;

                subcoordinators_type<TIME> subcoordinators() {
                	return _subcoordinators;
                }

                external_couplings<TIME> external_output_couplings() {
                	return _external_output_couplings;
                }

                external_couplings<TIME> external_input_couplings() {
                	return _external_input_couplings;
                }

                internal_couplings<TIME> subcoordinators_internal_couplings() {
                	return _internal_coupligns;
                }

                /**
                 * @brief Collects outputs ready for output before advancing the simulation
                 * @param t time the simulation will be advanced to
                 * @todo At this point all messages are copied while routed from one level to the next, need to find a good
                 * strategy to lower copying, maybe.
                 * @todo Merge the Collect output calls into the advance simulation as done with ICs and EICs routing
                 */
                void collect_outputs(const TIME &t) override {
                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_collect>(t, _model_id);

                    //collecting if necessary
                    if (_next < t) {
                        throw std::domain_error("Trying to obtain output when not internal event is scheduled");
                    } else if (_next == t) {
                        //log EOC
                        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eoc_collect>(t, _model_id);

                        // Fill all outboxes and clean the inboxes in the lower levels recursively
						#ifdef CADMIUM_EXECUTE_CONCURRENT
                        cadmium::dynamic::engine::collect_outputs_in_subcoordinators<TIME>(t, _subcoordinators, _threadpool);
						#else
							#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL
                        	cadmium::dynamic::engine::collect_outputs_in_subcoordinators<TIME, LOGGER>(t, _subcoordinators, _thread_number);
							#else
                        	cadmium::dynamic::engine::collect_outputs_in_subcoordinators<TIME>(t, _subcoordinators);
							#endif
						#endif

                        // Use the EOC mapping to compose current level output
                        _outbox = cadmium::dynamic::engine::collect_messages_by_eoc<TIME, LOGGER>(_external_output_couplings);
                    }
                }


				//#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL

                template<typename Iterator>
                std::vector<cadmium::parallel::info_for_logging<TIME>> collect_outputs_in_subcoordinators(Iterator first, Iterator last, const TIME &t) {
                	std::vector<cadmium::parallel::info_for_logging<TIME>> log_info;
                	cadmium::parallel::info_for_logging<TIME> result;

                	auto collect_output = [&t](auto &c)->std::vector<cadmium::parallel::info_for_logging<TIME>>
    				{
            			std::vector<cadmium::parallel::info_for_logging<TIME>> logs;
    					//cadmium::parallel::info_for_logging<TIME> result = c->collect_outputs_without_logging(t);
    					//return result;
    					logs.push_back(c->collect_outputs_without_logging(t));
    					return logs;
    				};


                	for(; first!=last;first++){
                		//result = _subcoordinators.at(i).collect_outputs_without_logging(t);
                		//result = (*first)->collect_outputs_without_logging(t);
                		result = _subcoordinators.at(first)->collect_outputs_without_logging(t);
                		log_info.push_back(result);
                	}

                	//log_info = cadmium::parallel::for_each_with_result<cadmium::parallel::info_for_logging<TIME>>(first, last, collect_output);

                	return log_info;
                }


                //#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL
                template<typename Iterator>
                std::vector<cadmium::dynamic::logger::routed_messages> internal_couplings_routing(Iterator first, Iterator last) {
                	std::vector<cadmium::dynamic::logger::routed_messages> log_info;
                	std::vector<cadmium::dynamic::logger::routed_messages> partial_logs;

                	auto route_messages = [](auto & c)->std::vector<cadmium::dynamic::logger::routed_messages>
                	{
                		//cadmium::parallel::info_for_logging<TIME>
                		std::vector<cadmium::dynamic::logger::routed_messages> logs;

                		for (const auto& l : c.second) {
                			auto& from_outbox = c.first.first->outbox();
                			auto& to_inbox = c.first.second->inbox();
                			cadmium::dynamic::logger::routed_messages message_to_log = l->route_messages(from_outbox, to_inbox);
                			logs.push_back(message_to_log);
                			//LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_collect>(message_to_log.from_port, message_to_log.to_port, message_to_log.from_messages, message_to_log.to_messages);
                		}
                	    return logs;
                	};

                	for(; first!=last; first++){
                		//result = _subcoordinators.at(i).collect_outputs_without_logging(t);
                		//partial_logs = first.route_messages();
                		partial_logs = route_messages(_internal_coupligns.at(first));
                		log_info.insert(log_info.end(), std::make_move_iterator(partial_logs.begin()), std::make_move_iterator(partial_logs.end()));
                	}

                	return log_info;
                }

                //#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL
                template<typename Iterator>
                std::vector<cadmium::parallel::info_for_logging<TIME>> apply_delta_without_logging(Iterator first, Iterator last, const TIME &t) {
                	std::vector<cadmium::parallel::info_for_logging<TIME>> log_info;
                	cadmium::parallel::info_for_logging<TIME> result;

                	for(; first!=last;first++){
                		//result = _subcoordinators.at(first)->advance_simulation_without_logging(t);
                		result = _subcoordinators.at(first)->advance_simulation_without_logging(t);
                		//result = first->advance_simulation_without_logging(t);
                		log_info.push_back(result);
                	}
                	return log_info;
                }


                //#if defined CPU_PARALLEL || defined CPU_MIN_PARALLEL
	            //template<typename TIME>
	            TIME min_next_in_subcoordinators(const subcoordinators_type<TIME>& subcoordinators, size_t thread_number) {
	            	std::vector<TIME> next_times(subcoordinators.size());
	            	std::transform(
	            			subcoordinators.cbegin(),
							subcoordinators.cend(),
							next_times.begin(),
							[] (const auto& c) -> TIME { return c->next(); }
	            	);
	            	//return *cadmium::parallel::parallel_min_element(next_times.begin(), next_times.end(), thread_number);
	            	return *std::min_element(next_times.begin(), next_times.end());
	            }


                //#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL

                template<typename Iterator>
                TIME next_in_subcoordinators(Iterator first, Iterator last) {
                	//std::vector<TIME> next_times(last-first);
                	//size_t n = std::distance(first, last);
                	//std::vector<TIME> next_times(n);
                	/*
                	std::transform(
	            			subcoordinators.cbegin(),
							subcoordinators.cend(),
							next_times.begin(),
							[] (const auto& c) -> TIME { return c->next(); }
	            	);
	            	*/
                /*
                	std::transform(
	            			first,
							last,
							next_times.begin(),
							[] (const auto& c) -> TIME { return c->next(); }
	            	);
	            */
                	size_t n = last-first;
                	TIME min = _subcoordinators.at(first)->next();
                	//TIME min;




                	for(; first!=last; first++){
                		if(_subcoordinators.at(first)->next() < min){
                			min = _subcoordinators.at(first)->next();

                			//std::cout << "NEXT: " << _subcoordinators.at(first)->next() << min << std::endl;

                		}
                	}


                	//std::cout << "AFTER LOOP NEXT: " << _subcoordinators.at(0)->next() << min << std::endl;


                	return min;
                	//return *std::min_element(next_times.begin(), next_times.end());
                }


                //#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                cadmium::parallel::info_for_logging<TIME> collect_outputs_without_logging(const TIME &t) {
                	cadmium::parallel::info_for_logging<TIME> log;

                	LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_collect>(t, _model_id);

                	//log.time = t;
                	//log.model_id = _model_id;
                	//collecting if necessary
                	if (_next < t) {
                		throw std::domain_error("Trying to obtain output when not internal event is scheduled");
                	} else if (_next == t) {
                		//log EOC
                		LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eoc_collect>(t, _model_id);
                		//log.imminent = true;
                		// Fill all outboxes and clean the inboxes in the lower levels recursively
                		#ifdef CADMIUM_EXECUTE_CONCURRENT
                		cadmium::dynamic::engine::collect_outputs_in_subcoordinators<TIME>(t, _subcoordinators, _threadpool);
                		#else
                			#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL
                			cadmium::dynamic::engine::collect_outputs_in_subcoordinators<TIME, LOGGER>(t, _subcoordinators, _thread_number);
                			#else
                			cadmium::dynamic::engine::collect_outputs_in_subcoordinators<TIME>(t, _subcoordinators);
                			#endif
                		#endif

                		// Use the EOC mapping to compose current level output
                		_outbox = cadmium::dynamic::engine::collect_messages_by_eoc<TIME, LOGGER>(_external_output_couplings);
                	}
                	return log;
                }
				//#endif //CPU_PARALLEL


                /**
                 * @brief outbox keeps the output generated by the last call to collect_outputs
                 */
                cadmium::dynamic::message_bags& outbox() override {
                    return _outbox;
                }

                /**
                 * @brief allows to access by reference the internal _inbox member
                 *
                 */
                cadmium::dynamic::message_bags& inbox() override {
                    return _inbox;
                }

                /**
                 * @brief advanceSimulation advances the execution to t, at t introduces the messages into the system (if any).
                 * @param t is the time the transition is expected to be run.
                 */
                void advance_simulation(const TIME &t) override {
                    //clean outbox because messages are routed before calling this function at a higher level
                    _outbox = cadmium::dynamic::message_bags();

                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_advance>(_last, t, _model_id);

                    if (_next < t || t < _last ) {
                        throw std::domain_error("Trying to obtain output when out of the advance time scope");
                    } else {

                        //Route the messages standing in the outboxes to mapped inboxes following ICs and EICs
                        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_ic_collect>(t, _model_id);
                        //cadmium::dynamic::engine::route_internal_coupled_messages_on_subcoordinators<TIME, LOGGER>(_internal_coupligns);

						#if defined CPU_PARALLEL || defined CPU_ROUTING_PARALLEL
                        	cadmium::dynamic::engine::route_internal_coupled_messages_on_subcoordinators<TIME, LOGGER>(_internal_coupligns, _thread_number);
						#else
                        	cadmium::dynamic::engine::route_internal_coupled_messages_on_subcoordinators<TIME, LOGGER>(_internal_coupligns);
						#endif


                        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eic_collect>(t, _model_id);
                        cadmium::dynamic::engine::route_external_input_coupled_messages_on_subcoordinators<TIME, LOGGER>(_inbox, _external_input_couplings);

                        //recurse on advance_simulation
						#ifdef CADMIUM_EXECUTE_CONCURRENT
                        cadmium::dynamic::engine::advance_simulation_in_subengines<TIME>(t, _subcoordinators, _threadpool);
						#else
							#if defined CPU_PARALLEL || defined CPU_DELTA_PARALLEL
                        	cadmium::dynamic::engine::advance_simulation_in_subengines<TIME, LOGGER>(t, _subcoordinators, _thread_number);
							#else
                        	cadmium::dynamic::engine::advance_simulation_in_subengines<TIME>(t, _subcoordinators);
							#endif
						#endif

                        //set _last and _next
                        _last = t;
                        //_next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);
						#if defined CPU_PARALLEL || defined CPU_MIN_PARALLEL
                        	_next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators, _thread_number);
                        	//_next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);
						#else
                        	_next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);
						#endif


                        //clean inbox because they were processed already
                        _inbox = cadmium::dynamic::message_bags();
                    }
                }

				//#if defined CPU_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                cadmium::parallel::info_for_logging<TIME> advance_simulation_without_logging(const TIME &t) {
                	cadmium::parallel::info_for_logging<TIME> log;
                	//clean outbox because messages are routed before calling this function at a higher level
                    _outbox = cadmium::dynamic::message_bags();

                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_advance>(_last, t, _model_id);

                    if (_next < t || t < _last ) {
                        throw std::domain_error("Trying to obtain output when out of the advance time scope");
                    } else {

                        //Route the messages standing in the outboxes to mapped inboxes following ICs and EICs
                        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_ic_collect>(t, _model_id);
                        //cadmium::dynamic::engine::route_internal_coupled_messages_on_subcoordinators<TIME, LOGGER>(_internal_coupligns);

						#if defined CPU_PARALLEL || defined CPU_ROUTING_PARALLEL
                        	cadmium::dynamic::engine::route_internal_coupled_messages_on_subcoordinators<TIME, LOGGER>(_internal_coupligns, _thread_number);
							#else
                        	cadmium::dynamic::engine::route_internal_coupled_messages_on_subcoordinators<TIME, LOGGER>(_internal_coupligns);
						#endif


                        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eic_collect>(t, _model_id);
                        cadmium::dynamic::engine::route_external_input_coupled_messages_on_subcoordinators<TIME, LOGGER>(_inbox, _external_input_couplings);

                        //recurse on advance_simulation
						#ifdef CADMIUM_EXECUTE_CONCURRENT
                        cadmium::dynamic::engine::advance_simulation_in_subengines<TIME>(t, _subcoordinators, _threadpool);
						#else
							#if defined CPU_PARALLEL || defined CPU_DELTA_PARALLEL
                        	cadmium::dynamic::engine::advance_simulation_in_subengines<TIME, LOGGER>(t, _subcoordinators, _thread_number);
							#else
                        	cadmium::dynamic::engine::advance_simulation_in_subengines<TIME>(t, _subcoordinators);
							#endif
						#endif

                        //set _last and _next
                        _last = t;
                        //_next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);

						#if defined CPU_PARALLEL || defined CPU_MIN_PARALLEL
                        	_next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators, _thread_number);
                        	//_next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);
						#else
                        	_next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);
						#endif


                        //clean inbox because they were processed already
                        _inbox = cadmium::dynamic::message_bags();
                    }

                    return log;
                }
				//#endif //CPU_PARALLEL
            };
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_COORDINATOR_HPP
