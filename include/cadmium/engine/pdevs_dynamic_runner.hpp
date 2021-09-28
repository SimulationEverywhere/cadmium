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
#include<limits>

#ifdef CADMIUM_EXECUTE_CONCURRENT
#include <boost/thread/executors/basic_thread_pool.hpp>
#endif //CADMIUM_EXECUTE_CONCURRENT

#if defined CPU_PARALLEL
#include <thread>
#include <cadmium/engine/parallel_helpers.hpp>
#endif //CPU_PARALLEL

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

                bool progress_bar = false;

                cadmium::dynamic::engine::coordinator<TIME, LOGGER> _top_coordinator; //this only works for coupled models.

                #ifdef CADMIUM_EXECUTE_CONCURRENT
                boost::basic_thread_pool _threadpool;
                #endif //CADMIUM_EXECUTE_CONCURRENT

                #ifdef CPU_PARALLEL
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
                    #if defined CPU_PARALLEL
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
                TIME run_until(const TIME &t) {
                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Starting run");
                    while (_next < t) {
                        LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(_next);
                        _top_coordinator.collect_outputs(_next);
                        _top_coordinator.advance_simulation(_next);
                        _next = _top_coordinator.next();

                        if (progress_bar)
                            progress_bar_meter(_next, t);
                    }

                    turn_progress_off();
                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Finished run");
                    return _next;
                }

                /**
                 * @brief runUntilPassivate starts the simulation and stops when there is no next internal event to happen.
                 */
                void run_until_passivate() {
                    run_until(std::numeric_limits<TIME>::infinity());
                }

                /**
                 * @brief Displays current progress of simulation
                 *  e.g., [50/500]
                 * 
                 * @param current Current time step
                 * @param total Maximum timestep (can be infinity)
                 */
                void progress_bar_meter(TIME current, TIME total)
                {
                    std::cout << "\r[" << current << "/";

                    if (total == std::numeric_limits<TIME>::infinity())
                        std::cout << "inf]";
                    else
                        std::cout << total << "]";

                    std::cout << std::flush;
                }

                void turn_progress_on()  { progress_bar = true;  std::cout << "\033[33m" << std::flush;  }
                void turn_progress_off() { progress_bar = false; std::cout << "\033[0m"  << std::flush;  }
            };
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_RUNNER_HPP
