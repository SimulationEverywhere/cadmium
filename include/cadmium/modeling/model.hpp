/**
 * Copyright (c) 2017, Laouen M. L. Belloli
 * Carleton University, Universidad de Buenos Aires
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

#ifndef CADMIUM_ATOMIC_HPP
#define CADMIUM_ATOMIC_HPP

namespace cadmium {
    namespace modeling {

        /**
         * @brief Empty atomic model class to allow dynamic cast atomic model without knowing the m
         * model type. The only thing to know is the TIME, which is the same for all models.
         *
         * @note This class derives the model class to allow pointer based polymorphism between
         * atomic and coupled models
         *
         * @tparam TIME - The class representing the model time.
         */
        template<typename TIME>
        class atomic_model : model {
            virtual void internal_transition() = 0;
            virtual void external_transition(TIME e, cadmium::dynamic_message_bags dynamic_bags) = 0;
            virtual void confluence_transition(TIME e, cadmium::dynamic_message_bags dynamic_bags) = 0;
            virtual cadmium::dynamic_message_bags output() const = 0;
            virtual TIME time_advance() const = 0;
        };

        /**
         * @brief Empty class to allow pointer based polymorphism between classes derived from atomic.
         */
        class model { };
    }
}

#endif // CADMIUM_ATOMIC_HPP