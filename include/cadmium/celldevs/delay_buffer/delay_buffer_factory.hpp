/**
 * Copyright (c) 2020, Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
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

#ifndef CADMIUM_CELLDEVS_DELAYER_FACTORY_HPP
#define CADMIUM_CELLDEVS_DELAYER_FACTORY_HPP

#include <string>
#include <exception>
#include <memory>
#include <cadmium/celldevs/delay_buffer/delay_buffer.hpp>
#include <cadmium/celldevs/delay_buffer/inertial.hpp>
#include <cadmium/celldevs/delay_buffer/transport.hpp>
#include <cadmium/celldevs/delay_buffer/hybrid.hpp>


namespace cadmium::celldevs {

    template<typename T, typename S>
    class delay_buffer_factory {
    public:
        template<typename... Args>
        static std::unique_ptr<delay_buffer<T, S>> create_delay_buffer(std::string const &delay_buffer_id, Args&&... args) {
            delay_buffer<T, S> *buffer_p = NULL;
            if (delay_buffer_id == "inertial")
                buffer_p = new inertial_delay_buffer<T, S>(std::forward<Args>(args)...);
            else if (delay_buffer_id == "transport")
                buffer_p = new transport_delay_buffer<T, S>(std::forward<Args>(args)...);
            else if (delay_buffer_id == "hybrid")
                buffer_p = new hybrid_delay_buffer<T, S>(std::forward<Args>(args)...);
            else throw std::out_of_range("Output delay buffer type not found");

            return std::unique_ptr<delay_buffer<T, S>>(buffer_p);
        }
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_DELAYER_FACTORY_HPP
