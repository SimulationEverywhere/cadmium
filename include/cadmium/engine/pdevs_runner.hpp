/**
 * Copyright (c) 2013-2017, Damian Vicino
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

#ifndef CADMIUM_PDEVS_RUNNER_HPP
#define CADMIUM_PDEVS_RUNNER_HPP
#include <iostream>
#include <cadmium/engine/pdevs_coordinator.hpp>
#include <cadmium/concept/atomic_model_assert.hpp>
#include <cadmium/logger/logger.hpp>
#include <cadmium/logger/common_loggers.hpp>


namespace cadmium {
    namespace engine {
        /**
         * @brief The Runner class runs the simulation.
         * The runner is in charge of setting up the coordinators and simulators, the initial
         * conditions, the ending conditions and the loggers, then it runs the simulation and
         * displays the results.
         *
         * @param Model The model to be simulated
         * @param Time Representation of time to be used to run the simualtion
         * @param Silent will not produce any output when running
         */

        //by default state changes get verbatim formatted and logged to cout
        using default_logger=cadmium::logger::logger<cadmium::logger::logger_state, cadmium::logger::verbatim_formater, cadmium::logger::cout_sink_provider>;

        //TODO: migrate specialization FEL behavior from CDBoost. At this point, there is no parametrized FEL.
        template <class TIME, template<class> class MODEL, typename LOGGER=default_logger>
        class runner{
            TIME _next; //next scheduled event

            //TODO: handle the case that the model received is an atomic model.
            cadmium::engine::coordinator<MODEL, TIME> top_coordinator; //this only works for coupled models.

        public:
            //contructors
            /**
             * @brief set the dynamic parameters for the simulation
             * @param init_time is the initial time of the simulation.
             */
            explicit runner(const TIME& init_time){
                LOGGER::template log<cadmium::logger::logger_global_time, TIME>(init_time);
                top_coordinator.init(init_time);
                _next = top_coordinator.next();
            }

            /**
             * @brief runUntil starts the simulation and stops when the next event is scheduled after t.
             * @param t is the limit time for the simulation.
             * @return the TIME of the next event to happen when simulation stopped.
             */
            TIME runUntil(const TIME& t) {
                while (_next < t){
                    LOGGER::template log<cadmium::logger::logger_global_time, TIME>(_next);
                    top_coordinator.advance_simulation(_next);
                    _next = top_coordinator.next();
                }
                return _next;
            }

            /**
             * @brief runUntilPassivate starts the simulation and stops when there is no next internal event to happen.
             */
            void runUntilPassivate() {
                while ( _next !=  std::numeric_limits<TIME>::infinity )
                {
                    LOGGER::template log<cadmium::logger::logger_global_time, TIME>(_next);
                    top_coordinator.advanceSimulation( _next);
                    _next = top_coordinator.next();
                }
            }
        };
    }
}


#endif // CADMIUM_PDEVS_RUNNER_HPP
