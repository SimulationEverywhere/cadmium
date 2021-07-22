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

#ifndef CADMIUM_PDEVS_DYNAMIC_FLAT_COORDINATOR_HPP
#define CADMIUM_PDEVS_DYNAMIC_FLAT_COORDINATOR_HPP

#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/engine/pdevs_dynamic_simulator.hpp>
#include <cadmium/engine/pdevs_dynamic_engine.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/dynamic_common_loggers.hpp>
#include <cadmium/engine/pdevs_dynamic_engine_helpers.hpp>
#include <cadmium/logger/common_loggers.hpp>

#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <string>
#include <cadmium/engine/pdevs_dynamic_link.hpp>

namespace cadmium {
    namespace dynamic {
        namespace engine {

            template<typename TIME, typename LOGGER>
            class flat_coordinator : public cadmium::dynamic::engine::engine<TIME> {

                //MODEL is assumed valid, the whole model tree is checked at "runner level" to fail fast
                TIME _last; //last transition time
                TIME _next; // next transition scheduled

                std::string _model_id;

                subcoordinators_type<TIME> _subcoordinators;
                external_couplings<TIME> _external_output_couplings;
                external_couplings<TIME> _external_input_couplings;
                internal_couplings<TIME> _internal_coupligns;

                subcoordinators_type<TIME> flat_subcoordinators;
                internal_couplings<TIME> flat_internal_couplings;

                std::map<std::string, std::shared_ptr<engine<TIME>>> engines_by_id;

                #ifdef CADMIUM_EXECUTE_CONCURRENT
                boost::basic_thread_pool* _threadpool;
                #endif //CADMIUM_EXECUTE_CONCURRENT

                #ifdef CPU_PARALLEL
                size_t _thread_number;
                #endif //CPU_PARALLEL

            public:

                dynamic::message_bags _inbox;
                dynamic::message_bags _outbox;

                using model_type = typename cadmium::dynamic::modeling::coupled<TIME>;

                /**
                 * @brief A dynamic coordinator must be constructed with the coupled model to coordinate.
                 */
                flat_coordinator() = delete;

                flat_coordinator(std::shared_ptr<model_type> coupled_model)
                        : _model_id(coupled_model->get_id())
                {
                    #ifdef CADMIUM_EXECUTE_CONCURRENT
                    _threadpool = nullptr;
                    #endif //CADMIUM_EXECUTE_CONCURRENT

                    //list of all coordinators and simulators in the structure
                    flat_subcoordinators = get_flat_subcoordinators(coupled_model);

                    //get alll couplings as internal ones
                    flat_internal_couplings = get_internal_couplings(flat_subcoordinators);

                    //remove intermediate levels
                    _internal_coupligns = remove_intermediate_levels(_internal_coupligns);
                }

                subcoordinators_type<TIME> get_flat_subcoordinators(std::shared_ptr<model_type> coupled_model) {
                	subcoordinators_type<TIME> _subcoordinators;
                	subcoordinators_type<TIME> partial_subcoordinators;

                	for(auto& m : coupled_model->_models) {
                    	partial_subcoordinators = get_subcoordinators(m);
                    	_subcoordinators.insert(_subcoordinators.end(), std::make_move_iterator(partial_subcoordinators.begin()), std::make_move_iterator(partial_subcoordinators.end()));
                    }

                	return _subcoordinators;
                }

                subcoordinators_type<TIME> get_subcoordinators(std::shared_ptr<cadmium::dynamic::modeling::model> model) {
                	subcoordinators_type<TIME> partial_subcoordinators;

                	std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::modeling::coupled<TIME>>(model);
                	std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> m_atomic = std::dynamic_pointer_cast<cadmium::dynamic::modeling::atomic_abstract<TIME>>(model);

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
                		for(auto& model : m_coupled->_models) {
                			partial_subcoordinators = get_subcoordinators(model);
                			_subcoordinators.insert(_subcoordinators.end(), std::make_move_iterator(partial_subcoordinators.begin()), std::make_move_iterator(partial_subcoordinators.end()));
                		}
                	}
                	engines_by_id.insert(std::make_pair(_subcoordinators.back()->get_model_id(), _subcoordinators.back()));
                	return _subcoordinators;
                }


                internal_couplings<TIME> get_internal_couplings(subcoordinators_type<TIME> flat_subcoordinators) {
                	internal_couplings<TIME> _partial_internal_couplings;
                	internal_couplings<TIME> _internal_couplings;
                	external_couplings<TIME> _external_output_couplings;
                	external_couplings<TIME> _external_input_couplings;
                	cadmium::dynamic::engine::internal_coupling<TIME> new_ic;

                	for(auto& element : flat_subcoordinators) {

                		std::shared_ptr<cadmium::dynamic::engine::coordinator<TIME,LOGGER>> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::engine::coordinator<TIME, LOGGER>>(element);

            			if(m_coupled != nullptr) {
            				_partial_internal_couplings  = m_coupled->get_internal_couplings();
        					_internal_couplings.insert(_internal_couplings.end(), std::make_move_iterator(_partial_internal_couplings.begin()), std::make_move_iterator(_partial_internal_couplings.end()));

        					_external_output_couplings = m_coupled->get_external_output_couplings();

        					for(auto& eoc : _external_output_couplings){
        						new_ic.first.first = eoc.first;
        						new_ic.first.second = engines_by_id.at(m_coupled->get_model_id());
        						new_ic.second = eoc.second;
        						_internal_coupligns.push_back(new_ic);
        					}

        					_external_input_couplings = m_coupled->get_external_input_couplings();

        					for(auto& eic : _external_input_couplings){
        						new_ic.first.first= engines_by_id.at(m_coupled->get_model_id());
        						new_ic.first.second = eic.first;
        						new_ic.second = eic.second;
        						_internal_coupligns.push_back(new_ic);
        					}
            			}
                	}
                	return _internal_couplings;
                }

                internal_couplings<TIME> remove_intermediate_levels(internal_couplings<TIME> internal_couplings) {
                	//std::type_index from_type, from_port_type, to_type, to_port_type;

                	for(auto& from : internal_couplings) {

                		std::shared_ptr<cadmium::dynamic::engine::engine<TIME>> engine;
                		engine = from.first.first;
                		std::shared_ptr<cadmium::dynamic::engine::coordinator<TIME,LOGGER>> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::engine::coordinator<TIME, LOGGER>>(from.first.first);

                		if(m_coupled != nullptr){
                			for(auto& to : internal_couplings){
                				if(from.first.first->get_model_id().compare(to.first.second->get_model_id()) == 0){
                					//for(std::vector<std::shared_ptr<cadmium::dynamic::engine::link_abstract>>::iterator link_from = from.second.begin() ; link_from != from.second.end(); ++link_from){
                					for(auto link_from = from.second.begin() ; link_from != from.second.end(); ++link_from){
                						//for(std::vector<std::shared_ptr<cadmium::dynamic::engine::link_abtract>>::iterator link_to = from.second.begin() ; link_to != from.second.end(); ++link_to){
                						for(auto link_to = from.second.begin() ; link_to != from.second.end(); ++link_to){
                							if((*link_from)->from_port_type_index() == (*link_to)->to_port_type_index()){
                								//std::type_index from_port_type, to_port_type;
                								//std::type_index  from_port_type = (*link_from)->from_type_index();
                								//std::type_index to_port_type = (*link_to)->to_type_index();

                								//using from_port=typename current_IC::from_model_output_port;

                								//std::type_index from_port = (*link_from)->from_port_type_index() ;
                								//std::type_index to_port = (*link_from)->to_port_type_index() ;

                								//std::type_index from_port = (*link_from)->from_port_type_index();
                								//std::type_index to_port = (*link_to)->to_port_type_index();

                								//std::type_index from_port = (*link_from)->from_port_type_index() ;
                								//std::type_index to_port = (*link_from)->to_port_type_index() ;

                								//using from_message_type = typename PORT_FROM::message_type;
                								//using from_message_bag_type = typename cadmium::message_bag<PORT_FROM>;
                								//using to_message_type = typename PORT_TO::message_type;
                								//using to_message_bag_type = typename cadmium::message_bag<PORT_TO>;

                								//std::shared_ptr<cadmium::dynamic::engine::link_abtract> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::modeling::coupled<TIME>>(m);

                								//std::type_index from_port_ = (*link_from)->from_port();
                								//std::type_index to_port_ = (*link_to)->to_port();

                								//decltype ((*link_from)->get_from_port();)

                								//typename from_port = ((*link_from)->from_port()).name();
                								//typename to_port = ((*link_to)->from_port_type_index()).name();

                								//std::string from_port = ((*link_from)->from_port());
                								//std::string to_port = ((*link_to)->to_port());

                								const std::type_info* from_port = ((*link_from)->from_port());
                								const std::type_info* to_port = ((*link_from)->to_port());

                								//boost::typeindex::type_id<T>().pretty_name();

                								//std::type_index(typeid(fruit::apple));

                								//struct from : public in_port<from_port_type>{};
                								//struct to : public out_port<to_port_type>{};



                								//std::shared_ptr<cadmium::dynamic::engine::link_abstract> ic_link = cadmium::dynamic::translate::make_link<from_port_type,to_port_type>();
                								//std::shared_ptr<cadmium::dynamic::engine::link_abstract> ic_link = cadmium::dynamic::translate::make_link<double, double>();

                								//if(from_port_type == to_port_type){
                									//cadmium::dynamic::modeling::IC ic_link = cadmium::dynamic::translate::make_IC<from_port,to_port>("sdfsdf","sdfsdff");
                								//}

                								//std::shared_ptr<cadmium::dynamic::engine::link_abstract> new_link = cadmium::dynamic::translate::make_link<from_port, to_port>();

                								//std::shared_ptr<cadmium::dynamic::engine::link_abstract> new_link = cadmium::dynamic::translate::make_link<decltype(from_port), decltype(to_port)>();
                								//boost::typeindex::type_id<PORT_FROM>()

                								Do(from_port, to_port);


                							}
                						}
                					}
                				}
                			}
                		}
                	}
                	return internal_couplings;
                }


                //template <typename T> void Do(T const &) {Do<T>();}

                template <typename T, typename P>
                void Do(T &, P &) {
                	std::shared_ptr<cadmium::dynamic::engine::link_abstract> new_link = cadmium::dynamic::translate::make_link<T, P>();

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
						#if defined CPU_PARALLEL
                    	cadmium::dynamic::engine::init_subcoordinators<TIME>(initial_time, _subcoordinators, _thread_number);
						#else
                    	cadmium::dynamic::engine::init_subcoordinators<TIME>(initial_time, _subcoordinators);
						#endif
                    #endif //CADMIUM_EXECUTE_CONCURRENT


                    //find the one with the lowest next time
                    _next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);
                }

                #ifdef CADMIUM_EXECUTE_CONCURRENT

                void init(TIME initial_time, boost::basic_thread_pool* threadpool) override {
                    _threadpool = threadpool;
                    this->init(initial_time);
                }

                #endif //CADMIUM_EXECUTE_CONCURRENT

				#ifdef CPU_PARALLEL
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
							#if defined CPU_PARALLEL
                        	cadmium::dynamic::engine::collect_outputs_in_subcoordinators<TIME>(t, _subcoordinators, _thread_number);
							#else
                        	cadmium::dynamic::engine::collect_outputs_in_subcoordinators<TIME>(t, _subcoordinators);
							#endif
						#endif

                        // Use the EOC mapping to compose current level output
                        _outbox = cadmium::dynamic::engine::collect_messages_by_eoc<TIME, LOGGER>(_external_output_couplings);
                    }
                }

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
                        cadmium::dynamic::engine::route_internal_coupled_messages_on_subcoordinators<TIME, LOGGER>(_internal_coupligns);

                        LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eic_collect>(t, _model_id);
                        cadmium::dynamic::engine::route_external_input_coupled_messages_on_subcoordinators<TIME, LOGGER>(_inbox, _external_input_couplings);

                        //recurse on advance_simulation
						#ifdef CADMIUM_EXECUTE_CONCURRENT
                        cadmium::dynamic::engine::advance_simulation_in_subengines<TIME>(t, _subcoordinators, _threadpool);
						#else
							#if defined CPU_PARALLEL
                        	cadmium::dynamic::engine::advance_simulation_in_subengines<TIME>(t, _subcoordinators, _thread_number);
							#else
                        	cadmium::dynamic::engine::advance_simulation_in_subengines<TIME>(t, _subcoordinators);
							#endif
						#endif

                        //set _last and _next
                        _last = t;
                        _next = cadmium::dynamic::engine::min_next_in_subcoordinators<TIME>(_subcoordinators);

                        //clean inbox because they were processed already
                        _inbox = cadmium::dynamic::message_bags();
                    }
                }
            };
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_FLAT_COORDINATOR_HPP
