/**
 * Copyright (c) 2013-2016, Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis
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


#ifndef CADMIUM_PDEVS_COORDINATOR_H
#define CADMIUM_PDEVS_COORDINATOR_H
#include <limits>
#include <boost/type_index.hpp>

#include <cadmium/engine/pdevs_engine_helpers.hpp>
#include <cadmium/modeling/coupling.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/concept/coupled_model_assert.hpp>
#include <cadmium/engine/pdevs_simulator.hpp>
#include <cadmium/logger/common_loggers.hpp>


namespace cadmium {
    namespace engine {
        /**
         * @brief The Coordinator class runs a PDEVS coupled model
         * The Coordinators are used to run the coupled models.
         * At the time the coupled model is assigned to the Coordinator it creates
         * other coordinators and simulators to handle the submodels of the coupled
         * model and then it coordinates the advance of all these coordinators and
         * simulators to provide its own outputs.
         * This kind of coordinator advances time by small certain steps.
         * There is never a rollback.
         * Each call to advanceSimulation advances internally a step and outputs are collected in separate method.
         */


            //TODO: migrate specialization FEL behavior from CDBoost. At this point, there is no parametrized FEL.

        template<template<typename T> class MODEL, typename TIME, typename LOGGER>
        class coordinator {

            //types for subcoordination
            template<typename P>
            using submodels_type=typename MODEL<TIME>::template models<P>;
            using in_bags_type=typename make_message_bags<typename MODEL<TIME>::input_ports>::type;
            using out_bags_type=typename make_message_bags<typename MODEL<TIME>::output_ports>::type;
            using subcoordinators_type=typename coordinate_tuple<TIME, submodels_type, LOGGER>::type;
            using eic=typename MODEL<TIME>::external_input_couplings;
            using eoc=typename MODEL<TIME>::external_output_couplings;
            using ic=typename MODEL<TIME>::internal_couplings;

            //MODEL is assumed valid, the whole model tree is checked at "runner level" to fail fast
            TIME _last; //last transition time
            TIME _next; // next transition scheduled
            subcoordinators_type _subcoordinators;

            //logging purposes
            std::string _model_id;

        public://making boxes temporarily public
            //TODO: set boxes back to private
            in_bags_type _inbox;
            out_bags_type _outbox;


        public:
            using model_type=MODEL<TIME>;
            /**
             * @brief init function sets the start time
             * @param t is the start time
             */
            void init(TIME t) noexcept {

                //logging data
                std::ostringstream oss;
                oss << boost::typeindex::type_id<MODEL<TIME>>().pretty_name();
                _model_id = oss.str();
                LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_init>(t, _model_id);

                _last = t;
                //init all subcoordinators and find next transition time.
                cadmium::engine::init_subcoordinators<TIME, subcoordinators_type>(t, _subcoordinators);
                //find the one with the lowest next time
                _next = cadmium::engine::min_next_in_tuple<subcoordinators_type>(_subcoordinators);
                return ;
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

                LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_collect>(t, _model_id);

                //collecting if necessary
                if (_next < t) {
                    throw std::domain_error("Trying to obtain output when not internal event is scheduled");
                } else if (_next == t) {
                    //log EOC
                    LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eoc_collect>(_model_id);
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

                LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::coor_info_advance>(_last, t, _model_id);

                if (_next < t || t < _last ) {
                    throw std::domain_error("Trying to obtain output when out of the advance time scope");
                } else {

                    //Route the messages standing in the outboxes to mapped inboxes following ICs and EICs
                    LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_ic_collect>(_model_id);
                    cadmium::engine::route_internal_coupled_messages_on_subcoordinators<TIME, subcoordinators_type, ic, LOGGER>(t, _subcoordinators);

                    LOGGER::template log<cadmium::logger::logger_message_routing, cadmium::logger::coor_routing_eic_collect>(_model_id);
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



#endif // CADMIUM_PDEVS_COORDINATOR_H
