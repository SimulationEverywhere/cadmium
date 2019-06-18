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

#ifndef CADMIUM_EMBEDDED_ERROR_HPP
#define CADMIUM_EMBEDDED_ERROR_HPP

#include <mbed.h>

namespace cadmium {
    namespace embedded {

        /**
         * @brief Wrapper for mbed error to indicate error inside Cadmium
         */
         class embedded_error {
            public:
                /**
                 * @brief Mbed Hard fault wrapper. Equivalent parameters to
                 * mbed error method.
                 */
                 void hard_fault() {}
                 template <typename H, typename... T> static void hard_fault(H p, T... t) {

                     //Construct Error string
                     std::string str ("\n\n++ RT_ARM_MBED Error Info ++ \n");
                     str += p;
                     str += "\n-- RT_ARM_MBED Error Info --\n\n";

                     //Call mbed error
                     error(str.c_str(), t...);
                 }
        };
    }
}

#endif //CADMIUM_RT_CLOCK_HPP
