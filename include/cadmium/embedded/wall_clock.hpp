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

          Timeout _timeout; //mbed timeout object
          bool expired;

          void timeout_expired() {
            expired = true;
          }

          //Return a long of time in microseconds
          long get_time_in_micro_seconds(const TIME &t) const {

            //Ignore Anything below 1 microsecond (Round down always)
            return t.getMicroseconds() +
               1000 * (t.getMilliseconds() +
                   1000 * (t.getSeconds()  +
                       60 * (t.getMinutes() +
                           60 * t.getHours()
                            )
                          )
                      );
           }

          //Given a long in microseconds, sleep the thread for that time
          void set_timeout(long time_left) {

            this->expired = false;

            //Handle waits of over ~35 minutes as timer overflows
            while (time_left > INT_MAX) {
              this->expired = false;
              this->_timeout.attach_us(callback(this, &wall_clock::timeout_expired), INT_MAX);
              time_left -= INT_MAX;

              while (!expired) sleep();
            }

            //Handle waits of under INT_MAX microseconds
            this->_timeout.attach_us(callback(this, &wall_clock::timeout_expired), time_left);
            while (!expired) sleep();

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
              while (1) sleep();

            //Don't wait
            } else if (t == TIME::zero()) {
              return;

            } else {
              set_timeout(get_time_in_micro_seconds(t));
            }

          }
        };
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_RUNNER_HPP
