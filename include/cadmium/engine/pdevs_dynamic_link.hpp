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

#ifndef CADMIUM_PDEVS_DYNAMIC_LINK_HPP
#define CADMIUM_PDEVS_DYNAMIC_LINK_HPP

#include <typeindex>
#include <memory>

#include <cadmium/logger/dynamic_common_loggers.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/common_loggers_helpers.hpp>

#include <cadmium/hpc_engine/parallel/locks.hpp>

#if defined CPU_PARALLEL || defined CPU_ROUTING_PARALLEL || defined GPU_PARALLEL || defined GPU_ROUTING_PARALLEL
#include <omp.h>
#include <stdexcept>
//#include <cadmium/hpc_engine/cpu_parallel/locks.hpp>
#endif //CPU_PARALLEL

namespace cadmium {
    namespace dynamic {
        namespace engine {

            class link_abstract {
            public:
                virtual std::type_index from_type_index() const = 0;

                virtual std::type_index from_port_type_index() const = 0;

                virtual std::type_index to_type_index() const = 0;

                virtual std::type_index to_port_type_index() const = 0;

                virtual cadmium::dynamic::logger::routed_messages
                route_messages(cadmium::dynamic::message_bags& bags_from, cadmium::dynamic::message_bags& bags_to) const = 0;

                virtual ~link_abstract() {}
            };

            template<typename PORT_FROM, typename PORT_TO>
            class link : public link_abstract {
            public:
                using from_message_type = typename PORT_FROM::message_type;
                using from_message_bag_type = typename cadmium::message_bag<PORT_FROM>;
                using to_message_type = typename PORT_TO::message_type;
                using to_message_bag_type = typename cadmium::message_bag<PORT_TO>;

                link() {
                    static_assert(
                            std::is_same<from_message_type, to_message_type>::value,
                            "PORT_FROM message type and PORT_TO message types must be the same type"
                    );
                }

                std::type_index from_type_index() const override {
                    return typeid(from_message_bag_type);
                }

                std::type_index from_port_type_index() const override {
                    return typeid(PORT_FROM);
                }

                std::type_index to_type_index() const override {
                    return typeid(to_message_bag_type);
                }

                std::type_index to_port_type_index() const override {
                    return typeid(PORT_TO);
                }

                cadmium::dynamic::logger::routed_messages
                pass_messages(const boost::any& bag_from, boost::any& bag_to) const {

					//#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                	//omp_init_lock(&writelock);
					//#endif //CPU_PARALLEL

                    from_message_bag_type b_from = boost::any_cast<from_message_bag_type>(bag_from);
                    to_message_bag_type *b_to = boost::any_cast<to_message_bag_type>(&bag_to);

                    #if defined CPU_PARALLEL || defined CPU_ROUTING_PARALLEL || defined GPU_PARALLEL || defined GPU_ROUTING_PARALLEL
                    //cadmium::dynamic::hpc_engine::cpu_parallel::locks l;
                    //cadmium::dynamic::hpc_engine::cpu_parallel::set_route_lock(this->to_port_type_index());

                    b_to->messages.insert(b_to->messages.end(), b_from.messages.begin(),
                                          b_from.messages.end());

/*
                    for (auto it = b_from.messages.begin() ; it != b_from.messages.end(); it++){
						b_to->messages.push_back(*it);
					}
*/
                    //cadmium::dynamic::hpc_engine::cpu_parallel::release_route_lock(this->to_port_type_index());
                    #else
                    b_to->messages.insert(b_to->messages.end(), b_from.messages.begin(),
                                                              b_from.messages.end());
                    #endif //CPU_PARALLEL

                    return cadmium::dynamic::logger::routed_messages(
                            cadmium::logger::messages_as_strings(b_from.messages),
                            cadmium::logger::messages_as_strings(b_to->messages),
                            boost::typeindex::type_id<PORT_FROM>().pretty_name(),
                            boost::typeindex::type_id<PORT_TO>().pretty_name()
                    );
                }

                cadmium::dynamic::logger::routed_messages
                pass_messages_to_new_bag(const boost::any& bag_from,
                                         cadmium::dynamic::message_bags& bags_to) const {
                    from_message_bag_type b_from = boost::any_cast<from_message_bag_type>(bag_from);
                    to_message_bag_type b_to;

    	            #if defined CPU_PARALLEL || defined CPU_ROUTING_PARALLEL || defined GPU_PARALLEL || defined GPU_ROUTING_PARALLEL
                    //cadmium::dynamic::hpc_engine::cpu_parallel::locks l;
                    //cadmium::dynamic::hpc_engine::cpu_parallel::set_route_lock(this->to_port_type_index());

                    b_to.messages.insert(b_to.messages.end(), b_from.messages.begin(),
                                         b_from.messages.end());
                    bags_to[this->to_port_type_index()] = b_to;

                    //cadmium::dynamic::hpc_engine::cpu_parallel::release_route_lock(this->to_port_type_index());
                    #else
                    b_to.messages.insert(b_to.messages.end(), b_from.messages.begin(),
                                                             b_from.messages.end());
                    bags_to[this->to_port_type_index()] = b_to;
                    #endif //CPU_PARALLEL

                    return cadmium::dynamic::logger::routed_messages(
                            cadmium::logger::messages_as_strings(b_from.messages),
                            cadmium::logger::messages_as_strings(b_to.messages),
                            boost::typeindex::type_id<PORT_FROM>().pretty_name(),
                            boost::typeindex::type_id<PORT_TO>().pretty_name()
                    );
                }

                /**
                 * @note This methods assumes the port is defined in the message_bags parameter bag,
                 * if is not the case, the function throws a std::map out of range exception.
                 *
                 * @param bags - The cadmium::dynamic::message_bags to check if there is messages in the from port
                 * @return true if there is messages, otherwise false
                 */
                bool is_there_messages_to_route(const cadmium::dynamic::message_bags &bags) const {
                    return boost::any_cast<from_message_bag_type>(
                            bags.at(this->from_port_type_index())).messages.size() > 0;
                }

                bool bag_not_empty(const cadmium::dynamic::message_bags &bags) const {
                	return boost::any_cast<to_message_bag_type>(
                			bags.at(this->to_port_type_index())).messages.size() > 0;
                }


                bool bag_exists (const cadmium::dynamic::message_bags &bags) const {
                	return boost::any_cast<to_message_bag_type>(
                			bags.at(this->to_port_type_index())) != NULL;
                }


                cadmium::dynamic::logger::routed_messages
                route_messages(cadmium::dynamic::message_bags& bags_from, cadmium::dynamic::message_bags& bags_to) const override {

                boost::any bag_from,bag_to;
                bool from_out_of_range = false, to_out_of_range=false;


					//#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                	//omp_init_lock(&writelock);
                	//omp_set_lock(&writelock);
					//#endif //CPU_PARALLEL

                	if (bags_from.find(this->from_port_type_index()) != bags_from.cend()) {

                        if (bags_to.find(this->to_port_type_index()) != bags_to.cend()) {
                		    return this->pass_messages(bags_from.at(this->from_port_type_index()),
                	                                   bags_to.at(this->to_port_type_index()));
                		}

                		if (this->is_there_messages_to_route(bags_from)) {
                			return this->pass_messages_to_new_bag(bags_from.at(this->from_port_type_index()), bags_to);
                		}
                	}

					//#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                    //omp_unset_lock(&writelock);
                    //omp_destroy_lock(&writelock);
					//#endif //CPU_PARALLEL


/*
                	try {
                		bag_from = bags_from.at(this->from_port_type_index());
                	} catch (const std::out_of_range& oor) {
                		from_out_of_range = true;
                	}

                	try {
                		bag_to = bags_from.at(this->to_port_type_index());
                	} catch (const std::out_of_range& oor) {
                		to_out_of_range = true;
                	}

                	if(from_out_of_range == false){
                		if(to_out_of_range == false){
                			return this->pass_messages(bags_from.at(this->from_port_type_index()),
                			                	       bags_to.at(this->to_port_type_index()));
                		}
                		if (this->is_there_messages_to_route(bags_from)) {
                			return this->pass_messages_to_new_bag(bags_from.at(this->from_port_type_index()), bags_to);
                		}
                	}
*/

/*
                	if (bags_from.at(this->to_port_type_index()).empty()) {
                		if (bags_to.find(this->to_port_type_index()) != bags_to.cend()) {
                			return this->pass_messages(bags_from.at(this->from_port_type_index()),
                	                	               bags_to.at(this->to_port_type_index()));
                	     }

                		if (this->is_there_messages_to_route(bags_from)) {
                			return this->pass_messages_to_new_bag(bags_from.at(this->from_port_type_index()), bags_to);
                	    }
                	}
*/



/*
                	if (bags_from.find(this->from_port_type_index()) != bags_from.cend()) {

                        if (bags_to.find(this->to_port_type_index()) != bags_to.cend()) {
                            return this->pass_messages(bags_from.at(this->from_port_type_index()),
                                                       bags_to.at(this->to_port_type_index()));
                        }

                        if (this->is_there_messages_to_route(bags_from)) {
							#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                        	omp_init_lock(&writelock);
                            omp_set_lock(&writelock);
							#endif //CPU_PARALLEL
                            return this->pass_messages_to_new_bag(
                                    bags_from.at(this->from_port_type_index()), bags_to);
							#if defined CPU_PARALLEL || defined CPU_LAMBDA_PARALLEL || defined CPU_DELTA_PARALLEL || defined CPU_ROUTING_PARALLEL || defined CPU_MIN_PARALLEL
                            omp_unset_lock(&writelock);
                            omp_destroy_lock(&writelock);
							#endif //CPU_PARALLEL
                        }

                    }
*/

/*
                	if (bags_from[this->from_port_type_index()] !=  0) {
                		if (bags_to[this->to_port_type_index()] != 0) {
                			return this->pass_messages(bags_from.at(this->from_port_type_index()),
                	                                   bags_to.at(this->to_port_type_index()));
                		}

                		if (this->is_there_messages_to_route(bags_from)) {
                			return this->pass_messages_to_new_bag(bags_from.at(this->from_port_type_index()), bags_to);
                		}
                	}
*/


/*
                    if (this->is_there_messages_to_route(bags_from)) {
                    	if (bags_to.find(this->to_port_type_index()) != bags_to.cend()) {
                    		return this->pass_messages(bags_from.at(this->from_port_type_index()),
                    								   bags_to.at(this->to_port_type_index()));
                    	}
                    	else{
                    		return this->pass_messages_to_new_bag(
                    				bags_from.at(this->from_port_type_index()), bags_to);
                    	}

                    }
*/

/*
                    if (this->is_there_messages_to_route(bags_from)) {
                    	if (bags_to.find(this->to_port_type_index()) != bags_to.cend()) {
                    		return this->pass_messages(bags_from.at(this->from_port_type_index()),
                    									bags_to.at(this->to_port_type_index()));
                    	}
                    }
*/

/*
                	//if (this->is_there_messages_to_route(bags_from)) {
                		if (bags_from.find(this->from_port_type_index()) != bags_from.cend()) {
                			if (bags_to.find(this->to_port_type_index()) != bags_to.cend()) {
                				return this->pass_messages(bags_from.at(this->from_port_type_index()),
                	                                   bags_to.at(this->to_port_type_index()));
                			}
                		}
                	//}
*/


                    cadmium::dynamic::logger::routed_messages empty_ret(
                            boost::typeindex::type_id<PORT_FROM>().pretty_name(),
                            boost::typeindex::type_id<PORT_TO>().pretty_name()
                    );
                    return empty_ret; // if no messages where routed, it returns an empty vector
                }
            };
        }
    }
}


#endif //CADMIUM_PDEVS_DYNAMIC_LINK_HPP
