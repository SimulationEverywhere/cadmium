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
#include <cadmium/celldevs/delayer/delayer.hpp>
#include <cadmium/celldevs/delayer/inertial.hpp>
#include <cadmium/celldevs/delayer/transport.hpp>


namespace cadmium::celldevs {
    /**
     * @brief Abstract factory for creating Cell-DEVS output delayer objects.
     * @tparam T the type used for representing time in a simulation.
     * @tparam S the type used for representing a cell state.
     * @see delayer/delayer.hpp
     */
    template <typename T, typename S>
    class delayer_factory {
    public:
        /**
         * @brief creates a Cell-DEVS output delayer object.
         * @tparam Args type of any additional parameter required for creating the output delayer.
         * @param delayer_name name or ID of the output delayer type.
         * @param args initialization parameters.
         * @return pointer to the requested output delayer.
         * @throw bad_typeid exception if output delayer type does not match with any output delayer.
         */
        template <typename... Args>
        static delayer<T, S> *create_delayer(std::string const &delayer_name, Args&&... args) {
            if (delayer_name == "inertial") {
                return new inertial_delayer<T, S>(std::forward<Args>(args)...);
            } else if (delayer_name == "transport") {
                return new transport_delayer<T, S>(std::forward<Args>(args)...);
            } else throw std::bad_typeid();
        }
    };
} //namespace cadmium::celldevs
#endif //CADMIUM_CELLDEVS_DELAYER_FACTORY_HPP
