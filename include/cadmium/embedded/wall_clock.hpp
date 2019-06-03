/**
 * Copyright (c) 2019, Kyle Bjornson
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

#ifndef CADMIUM_WALL_CLOCK_HPP
#define CADMIUM_WALL_CLOCK_HPP

#include <mbed.h>

namespace cadmium {
    namespace embedded {

        /**
         * @brief Wall Clock class used to delay execution and follow actual time.
         * Used mbed timeout, and attempts to sleep the main thread to save some power.
         */
        template<class TIME>
        class wall_clock {

        private:

          //Time since last time advance, how long the simulator took to advance
          Timer execution_timer;

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

           //Given a long in microseconds, sleep for that time
           void set_timeout(long delay_ms, int delay_remainder_us) {

            if (delay_ms > 0) {

              //Handle waits over 50 days if necessary.
              while (delay_ms > INT_MAX) {
                wait_ms(INT_MAX);
                delay_ms -= INT_MAX;
              }

              //Sleep for the remaining time
              wait_ms(delay_ms);

            }

            if (delay_remainder_us > 0) {
              wait_us(delay_remainder_us);
            }

          }


       public:

            /**
             * @brief wait_for delays simulation by given time
             * @param t is the time to delay
             * @return the TIME of the next event to happen when simulation stopped.
             */
          void wait_for(const TIME &t) {
            long actual_delay;

            //If negative time, halt and print error over UART
            MBED_ASSERT(t >= TIME::zero());

            //Wait forever
            if (t == TIME::infinity()) {
              while(1); //Sleep
            }

            execution_timer.stop();
            actual_delay = get_time_in_micro_seconds(t)
                                - execution_timer.read_us();

            if (actual_delay > 0) {
              set_timeout(actual_delay / 1000, actual_delay % 1000);
            } else {
              //Missed Real Time Deadline!!
              MBED_ASSERT(false); //Do something meaningful here.
            }

            execution_timer.reset();
            execution_timer.start();

          }
        };
    }
}

#endif //CADMIUM_WALL_CLOCK_HPP
