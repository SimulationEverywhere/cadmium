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

//SET RT_DEVS and load load the correct clock
#ifdef RT_ARM_MBED 
    #include <cadmium/real_time/arm_mbed/rt_clock.hpp>
#elif RT_LINUX
    #include <cadmium/real_time/linux/rt_clock.hpp>
#endif

#ifdef RT_DEVS
    #include <cadmium/modeling/dynamic_model.hpp>
    //Gross global boolean to say if an interrupt occured.
    //Todo: Do this better - can we avoid making it a global bool?
    //      Maybe have every enlist as an asynchronus atomic observer?
    // volatile bool interrupted = false;
    bool serviceInterrupts = false;
#endif

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
                TIME _last;
                TIME _next; //next scheduled event

                cadmium::dynamic::engine::coordinator<TIME, LOGGER> _top_coordinator; //this only works for coupled models.

                #ifdef CADMIUM_EXECUTE_CONCURRENT
                boost::basic_thread_pool _threadpool;
                #endif //CADMIUM_EXECUTE_CONCURRENT

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

                explicit runner(std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> coupled_model, const TIME &init_time)
                : _top_coordinator(coupled_model){
                    LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(init_time);
                    LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Preparing model");
                    _top_coordinator.init(init_time);
                    _next = _top_coordinator.next();

                }
                #endif //CADMIUM_EXECUTE_CONCURRENT

                /**
                 * @brief runUntil starts the simulation and stops when the next event is scheduled after t.
                 * @param t is the limit time for the simulation.
                 * @return the TIME of the next event to happen when simulation stopped.
                 */
                #ifdef RT_DEVS

                    TIME run_until(const TIME &t) {
                        TIME e;
                        TIME temp;
                        LOGGER::template log<cadmium::logger::logger_info, cadmium::logger::run_info>("Starting run");

                        _last = TIME();
                        cadmium::embedded::rt_clock<TIME> timer(_top_coordinator.get_async_subjects());

                        while (_next < t) {
                            if (_next != _last) {
                                e = timer.wait_for(_next - _last);
                                if(e == TIME::zero()){
                                    _last = _next;

                                } else {
                                    //interrupt occured, we must handle it.
                                    _last += e;
                                    _top_coordinator.interrupt_notify(_last);
                                    serviceInterrupts = true;
                                }
                            }
                            LOGGER::template log<cadmium::logger::logger_global_time, cadmium::logger::run_global_time>(_last);
                            _top_coordinator.collect_outputs(_last);
                            _top_coordinator.advance_simulation(_last);
                            _next = _top_coordinator.next();
                            if(serviceInterrupts) {
                                serviceInterrupts = false;
                            }
                        }
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
