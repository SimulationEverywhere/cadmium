/**
* Copyright (c) 2020, Román Cárdenas Rodríguez
* ARSLab - Carleton University
* GreenLSI - Polytechnic University of Madrid
* All rights reserved.
*
* Abstract factory for creating Cell-DEVS output delayer objects.
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
