/**
 * Copyright (c) 2019, Kyle Bjornson, Ben Earle
 * Carleton University
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

#ifndef CADMIUM_RT_CLOCK_HPP
#define CADMIUM_RT_CLOCK_HPP

#include <chrono>
#include <iostream>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/common_loggers.hpp>

static long MIN_TO_MICRO   = (1000*1000*60);
static long SEC_TO_MICRO   = (1000*1000);
static long MILI_TO_MICRO  = (1000);

#ifndef MISSED_DEADLINE_TOLERANCE
  #define MISSED_DEADLINE_TOLERANCE -1
#endif
extern volatile bool interrupted;
namespace cadmium {
    namespace embedded {

        class Timer
        {
          std::chrono::high_resolution_clock::time_point start_time, end_time;
        public:
          Timer():
            start_time( std::chrono::high_resolution_clock::now()){}

          void start() {
            start_time = std::chrono::high_resolution_clock::now();
          }
          void reset() {
            end_time = start_time;
          }
          void stop(){
            end_time = std::chrono::high_resolution_clock::now();
          }
          long live_read_us(){
            return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count() * SEC_TO_MICRO;            
          }
          long read_us(){
            return std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count() * SEC_TO_MICRO;
          }
        };

        /**
         * @brief Wall Clock class used to delay execution and follow actual time.
         * Used mbed timeout, and attempts to sleep the main thread to save some power.
         */
        template<class TIME, typename LOGGER=cadmium::logger::logger<cadmium::logger::logger_debug,
                                             cadmium::dynamic::logger::formatter<TIME>,
                                             cadmium::logger::cout_sink_provider>>
        class rt_clock : public cadmium::dynamic::modeling::AsyncEventObserver {
        private:

          //Time since last time advance, how long the simulator took to advance
          Timer execution_timer;

          // If the next event (actual_delay) is in the future AKA we are ahead of schedule it will be reset to 0
          // If actual_delay is negative we are behind schedule, in this case we will store how long behind schedule we are in scheduler_slip.
          // This is then added to the next actual delay and updated until we surpass the tolerance or recover from the slip.
          long scheduler_slip = 0;

          //Return a long of time in microseconds
          long get_time_in_micro_seconds(const TIME &t) const {

            //Ignore Anything below 1 microsecond
            return t.getMicroseconds() +
              1000 * (t.getMilliseconds() +
                1000 * (t.getSeconds() +
                    60 * (t.getMinutes() +
                      60 * t.getHours()
                         )
                       )
                     );
            }

          TIME micro_seconds_to_time(long us) const {
            int hours, min, sec, ms, microseconds;
            hours = us / MIN_TO_MICRO;
            hours /= 60;
            us = us % (MIN_TO_MICRO*60);

            min = us / MIN_TO_MICRO;
            us = us % MIN_TO_MICRO;

            sec = us / SEC_TO_MICRO;
            us = us % SEC_TO_MICRO;

            ms = us / MILI_TO_MICRO;
            microseconds = us % MILI_TO_MICRO;

            return TIME{hours, min, sec, ms, microseconds};
          }

          //Given a long in microseconds, sleep for that time
          long set_timeout(long delay_us) {
            execution_timer.reset();
            execution_timer.start();
            //Wait until timeout or interrupt
            while(!interrupted && (delay_us > execution_timer.live_read_us()));
            
            execution_timer.stop();
            if(interrupted) {
              return delay_us - execution_timer.read_us();
            }
            return delay_us;
          }


       public:

          rt_clock(std::vector < class cadmium::dynamic::modeling::AsyncEventSubject * > sub) : cadmium::dynamic::modeling::AsyncEventObserver(sub) {}

            /**
             * @brief wait_for delays simulation by given time
             * @param t is the time to delay
             * @return the TIME of the next event to happen when simulation stopped.
             */
          TIME wait_for(const TIME &t) {
            long actual_delay;

            //If negative time, halt and print error over UART
            assert(t >= TIME::zero());

            //Wait forever
            if (t == TIME::infinity()) {
              while(1); //Sleep
            }

            execution_timer.stop();
            actual_delay = get_time_in_micro_seconds(t)
                                - execution_timer.read_us()
                                + scheduler_slip;

            // Slip keeps track of how far behind schedule we are.
            scheduler_slip = actual_delay;
            // If we are ahead of schedule, then reset it to zero
            if (scheduler_slip >= 0) {
              scheduler_slip = 0;

            //Enable debug logs to see schedule slip
            } else {
              #ifdef DEBUG_SCHEDULING
                LOGGER::template log<cadmium::logger::logger_debug,cadmium::logger::run_info>
                                  ("MISSED SCHEDULED TIME ADVANCE! SLIP = " + to_string(-scheduler_slip) + " microseconds\n");
              #endif
            }

            if (MISSED_DEADLINE_TOLERANCE != -1 ) {
              if (actual_delay >= -MISSED_DEADLINE_TOLERANCE) {
                actual_delay = set_timeout(actual_delay);
              } else {
                //Missed Real Time Deadline and could not recover (Slip is passed the threshold)
                std::cout << "MISSED SCHEDULED TIME ADVANCE DEADLINE BY:" << -actual_delay << " microseconds \n";
                assert(false);
              }
            }

            execution_timer.reset();
            execution_timer.start();

            return micro_seconds_to_time(0);
          }
          void update(){
            interrupted = true;
          }
        };
    }
}

#endif //CADMIUM_RT_CLOCK_HPP
