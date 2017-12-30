/**
 * Copyright (c) 2017, Laouen M. L. Belloli
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

#ifndef CADMIUM_PDEVS_DYNAMIC_COORDINATOR_HPP
#define CADMIUM_PDEVS_DYNAMIC_COORDINATOR_HPP

#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/engine/pdevs_dynamic_simulator.hpp>
#include <cadmium/engine/pdevs_dynamic_engine.hpp>
#include <cadmium/modeling/dynamic_message_bag.hpp>

namespace cadmium {
    namespace dynamic {
        namespace engine {

            template<typename TIME, typename LOGGER>
            class coordinator {
                //TODO: implement correct coordinator formatter that does not depend on the MODEL template parameter
                //using formatter=typename cadmium::logger::coordinator_formatter<MODEL, TIME>;

                //types for subcoordination
                template<typename P>
//            using submodels_type=typename MODEL<TIME>::template models<P>;
//            using in_bags_type=typename make_message_bags<typename MODEL<TIME>::input_ports>::type;
//            using out_bags_type=typename make_message_bags<typename MODEL<TIME>::output_ports>::type;
//            using eic=typename MODEL<TIME>::external_input_couplings;
//            using eoc=typename MODEL<TIME>::external_output_couplings;
//            using ic=typename MODEL<TIME>::internal_couplings;
                using subcoordinators_type=typename std::vector<std::shared_ptr<engine<TIME>>>;

                //MODEL is assumed valid, the whole model tree is checked at "runner level" to fail fast
                TIME _last; //last transition time
                TIME _next; // next transition scheduled
                subcoordinators_type _subcoordinators;

                dynamic::message_bags _inbox;
                dynamic::message_bags _outbox;


            public:
                using model_type = typename cadmium::dynamic::modeling::coupled<TIME>;

                /**
                 * @brief A dynamic coordinator must be constructed with the coupled model to coordinate.
                 */
                coordinator() = delete;

                coordinator(model_type coupled_model) {

                    for(auto& m : coupled_model._models) {
                        std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> m_coupled = std::dynamic_pointer_cast<cadmium::dynamic::modeling::coupled<TIME>>(m.second);
                        std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> m_atomic = std::dynamic_pointer_cast<cadmium::dynamic::modeling::atomic_abstract<TIME>>(m.second);

                        if (m_coupled == nullptr) {
                            assert(m_atomic != nullptr);
                            auto simulator = std::make_shared<cadmium::dynamic::engine::simulator<TIME, LOGGER>>(m_atomic);
                            _subcoordinators.push_back(simulator);
                        } else {
                            auto coordinator = std::make_shared<coordinator<TIME, LOGGER>>(m_coupled);
                            _subcoordinators.push_back(coordinator);
                        }
                    }
                }

                /**
                 * @brief init function sets the start time
                 * @param t is the start time
                 */
                void init(TIME t) noexcept {
                    //LOGGER::template log<cadmium::logger::logger_info, decltype(formatter::log_info_init), TIME>(formatter::log_info_init, t);

                    _last = t;
                    //init all subcoordinators and find next transition time.

                    cadmium::engine::init_subcoordinators<TIME, subcoordinators_type>(t, _subcoordinators);
                    //find the one with the lowest next time
                    _next = cadmium::engine::min_next_in_tuple<subcoordinators_type>(_subcoordinators);
                }

                /**
                 * @brief Coordinator expected next internal transition time
                 */
                TIME next() const noexcept {
                    return _next;
                }

                /**
                 * @brief Collects outputs ready for output before advancing the simulation
                 * @param t time the simulation will be advanced to
                 * @todo At this point all messages are copied while routed form onelevel to the next, need to find a good
                 * strategy to lower copying, maybe.
                 * @todo Merge the Collect output calls into the advance simulation as done with ICs and EICs routing
                 */

                void collect_outputs(const TIME &t) {
                    //LOGGER::template log<cadmium::logger::logger_info, decltype(formatter::log_info_collect), TIME>(formatter::log_info_collect, t);

                    //collecting if necessary
                    if (_next < t) {
                        throw std::domain_error("Trying to obtain output when not internal event is scheduled");
                    } else if (_next == t) {
                        //log EOC
                        //LOGGER::template log<cadmium::logger::logger_message_routing, decltype(formatter::log_routing_collect)>(formatter::log_routing_collect);
                        //fill all outboxes and clean the inboxes in the lower levels recursively
                        cadmium::engine::collect_outputs_in_subcoordinators<TIME, subcoordinators_type>(t, _subcoordinators);
                        //use the EOC mapping to compose current level output
                        _outbox = collect_messages_by_eoc<TIME, eoc, out_bags_type, subcoordinators_type, LOGGER>(_subcoordinators);
                    }
                }

                /**
                 * @brief outbox keeps the output generated by the last call to collect_outputs
                 */
                out_bags_type outbox() const noexcept{
                    return _outbox;
                }

                /**
                 * @brief advanceSimulation advances the execution to t, at t introduces the messages into the system (if any).
                 * @param t is the time the transition is expected to be run.
                 */
                void advance_simulation(const TIME &t) {
                    //clean outbox because messages are routed before calling this funtion at a higher level
                    _outbox = out_bags_type{};

                    //LOGGER::template log<cadmium::logger::logger_info, decltype(formatter::log_info_advance), TIME>(formatter::log_info_advance, _last, t);

                    if (_next < t || t < _last ) {
                        throw std::domain_error("Trying to obtain output when out of the advance time scope");
                    } else {

                        //Route the messages standing in the outboxes to mapped inboxes following ICs and EICs
                        //LOGGER::template log<cadmium::logger::logger_message_routing, decltype(formatter::log_routing_ic_collect)>(formatter::log_routing_ic_collect);
                        cadmium::engine::route_internal_coupled_messages_on_subcoordinators<TIME, subcoordinators_type, ic, LOGGER>(t, _subcoordinators);

                        //LOGGER::template log<cadmium::logger::logger_message_routing, decltype(formatter::log_routing_eic_collect)>(formatter::log_routing_eic_collect);
                        cadmium::engine::route_external_input_coupled_messages_on_subcoordinators<TIME, in_bags_type, subcoordinators_type, eic, LOGGER>(t, _inbox, _subcoordinators);

                        //recurse on advance_simulation
                        cadmium::engine::advance_simulation_in_subengines<TIME, subcoordinators_type>(t, _subcoordinators);

                        //set _last and _next
                        _last = t;
                        _next = cadmium::engine::min_next_in_tuple<subcoordinators_type>(_subcoordinators);

                        //clean inbox because they were processed already
                        _inbox = in_bags_type{};
                    }
                }
            };
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_COORDINATOR_HPP
