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

           //Return a long of time in microseconds
           long get_time_in_milli_seconds(const TIME &t) const {

             //Ignore Anything below 1 millisecond
             return t.getMilliseconds() +
                    1000 * (t.getSeconds()  +
                        60 * (t.getMinutes() +
                            60 * t.getHours()
                             )
                           );
            }

          //Given a long in microseconds, sleep the thread for that time
          void set_timeout(long delay_ms, int delay_remainder_us) {

            //Use non blocking thread sleep for time advance > 1 ms
            //This allows other lower priority threads to execute
            if (delay_ms > 0) {

              //Handle waits over 50 days if necessary.
              while (delay_ms > INT_MAX) {
                ThisThread::sleep_for(INT_MAX);
                delay_ms -= INT_MAX;
              }

              //Sleep this thread for desired time
              ThisThread::sleep_for(delay_ms);

            }

            //Cannot sleep thread for less than 1 millisecond
            //Use blocking wait based on different timer instead
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

            //If negative time, halt and print error over UART
            MBED_ASSERT(t >= TIME::zero());

            //Wait forever
            if (t == TIME::infinity()) {
              while(1); //Sleep

            //Don't wait
            } else if (t == TIME::zero()) {
              return;

            } else {
              set_timeout(get_time_in_milli_seconds(t), t.getMicroseconds());
            }

          }
        };
    }
}

#endif //CADMIUM_WALL_CLOCK_HPP
