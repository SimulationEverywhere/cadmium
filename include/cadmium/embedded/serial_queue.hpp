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

#ifndef CADMIUM_SERIAL_QUEUE_HPP
#define CADMIUM_SERIAL_QUEUE_HPP

#include <mbed.h>

namespace cadmium {
    namespace embedded {

        //Only need one low priority thread for prints
        static Thread low_priority_thread(osPriorityLow);

        /**
         * @brief This class makes use of the MBED RTOS Thread features
         * by making serial printing happen in a lower priority thread.
         * This makes it less likely that (slow) UART printing can interfere
         * with the rest of the execution.
         */
        class serial_queue : public std::streambuf, public std::ostream {

          private:
            EventQueue serial_event_queue;

            //Serial print in lower priority thread
            static void low_prio_serial_print(std::streambuf::int_type c) {
              std::cout.put(c);
            }


          public:

            //Start low priority thread, run forever
            serial_queue() : std::ostream(this) {

              low_priority_thread.start(callback(&serial_event_queue,
                                          &EventQueue::dispatch_forever));

            }

            //streambuf overflow overrided to let low priority thread do the printing
            std::streambuf::int_type overflow(std::streambuf::int_type c) override {

              serial_event_queue.call(&low_prio_serial_print, c);

              return 0;
            }

        };
    }
}

#endif //CADMIUM_SERIAL_QUEUE_HPP
