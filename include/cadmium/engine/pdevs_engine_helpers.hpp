/**
 * Copyright (c) 2013-2017, Damian Vicino, Laouen M. L. Belloli
 * Carleton University, Universite de Nice-Sophia Antipolis, Universidad de Buenos Aires
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


#ifndef PDEVS_ENGINE_HELPERS_HPP
#define PDEVS_ENGINE_HELPERS_HPP

#include <type_traits>
#include <tuple>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <boost/type_index.hpp>

#include <cadmium/concept/concept_helpers.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include <cadmium/logger/common_loggers_helpers.hpp>
#include <cadmium/engine/common_helpers.hpp>


namespace cadmium {
    namespace engine {
        //forward declaration
        template<template<typename T> class MODEL, typename TIME, typename LOGGER>
        class coordinator;        //forward declaration
        template<template<typename T> class MODEL, typename TIME, typename LOGGER>
        class simulator;

        //finding the min next from a tuple of coordinators and simulators
        template<typename T, std::size_t S>
        struct min_next_in_tuple_impl {
            static auto value(T& t) {
                return std::min(std::get<S - 1>(t).next(), min_next_in_tuple_impl<T, S - 1>::value(t));
            }
        };

        template<typename T>
        struct min_next_in_tuple_impl<T, 1> {
            static auto value(T& t) {
                return std::get<0>(t).next();
            }
        };

        template<typename T>
        auto min_next_in_tuple(T& t) {
            return min_next_in_tuple_impl<T, std::tuple_size<T>::value>::value(t);
        }

        //We use COS to accumulate coordinators and simulators while iterating MT using the S index
        template<typename TIME, template<typename> class MT, std::size_t S, typename LOGGER, typename... COS> //COS accumulates coords or sims
        struct coordinate_tuple_impl {
            template<typename T>
            using current=typename std::tuple_element<S - 1, MT<T>>::type;
            using current_coordinated=typename std::conditional<cadmium::concept::is_atomic<current>::value(), simulator<current, TIME, LOGGER>, coordinator<current, TIME, LOGGER>>::type;
            using type=typename coordinate_tuple_impl<TIME, MT, S - 1, LOGGER, current_coordinated, COS...>::type;
        };

        //When the S reaches 0, all coordinators and simulators are put into a tuple for return
        template<typename TIME, template<typename> class MT, typename LOGGER, typename... COS>
        struct coordinate_tuple_impl<TIME, MT, 0, LOGGER, COS...> {
            using type=std::tuple<COS...>;
        };

        template<typename TIME, template<typename> class MT, typename LOGGER>
        struct coordinate_tuple {
            //the size should not be affected by the type used for TIME, simplifying passing float
            using type=typename coordinate_tuple_impl<TIME, MT, std::tuple_size<MT<float>>::value, LOGGER>::type;
        };

        //initialize subcoordinators
        template<typename TIME, typename CST>
        void init_subcoordinators(const TIME& t, CST& cs) {

            auto init_coordinator = [&t](auto & c)->void { c.init(t); };
            cadmium::helper::for_each<CST>(cs, init_coordinator);
        }

        //populate the outbox of every subcoordinator recursively
        template<typename TIME, typename CST>
        void collect_outputs_in_subcoordinators(const TIME& t, CST& cs) {

            auto collect_output = [&t](auto & c)->void { c.collect_outputs(t); };
            cadmium::helper::for_each<CST>(cs, collect_output);
        }

        //get the engine  from a tuple of engines that is simulating the model provided
        template<typename TIMED_MODEL, typename CST, size_t S>
        struct get_engine_by_model_impl{
            using current_engine=typename std::tuple_element<S-1, CST>::type;
            using current_model=typename current_engine::model_type;
            using type=typename std::conditional<std::is_same<current_model, TIMED_MODEL>::value, current_engine, typename get_engine_by_model_impl<TIMED_MODEL, CST, S-1>::type>::type;
        };

        struct NO_SIMULATOR{};

        template<typename TIMED_MODEL, typename CST>
        struct get_engine_by_model_impl<TIMED_MODEL, CST, 1>{
            using current_engine=typename std::tuple_element<0, CST>::type;
            using current_model=typename current_engine::model_type;
            using type=typename std::conditional<std::is_same<current_model, TIMED_MODEL>::value, current_engine, NO_SIMULATOR>::type;
        };

        template<typename TIMED_MODEL, typename CST>
        struct get_engine_type_by_model{
            using type=typename get_engine_by_model_impl<TIMED_MODEL, CST, std::tuple_size<CST>::value>::type;
        };

        template<typename TIMED_MODEL, typename CST>
        //typename get_engine_type_by_model<TIMED_MODEL, CST>::type
        typename get_engine_type_by_model<TIMED_MODEL, CST>::type & get_engine_by_model(CST& cst){
            return std::get<typename get_engine_type_by_model<TIMED_MODEL, CST>::type>(cst);
        }

        //map the messages in the outboxes of subengines to the messages in the outbox of current coordinator
        template<typename TIME, typename EOC, std::size_t S, typename OUT_BAG, typename CST, typename LOGGER>
        struct collect_messages_by_eoc_impl{
            using external_output_port=typename std::tuple_element<S-1, EOC>::type::external_output_port;
            using submodel_from = typename std::tuple_element<S-1, EOC>::type::template submodel<TIME>;
            using submodel_output_port=typename std::tuple_element<S-1, EOC>::type::submodel_output_port;
            using submodel_out_messages_type=typename make_message_bags<typename std::tuple<submodel_output_port>>::type;

            static void fill(OUT_BAG& messages, CST& cst){
                //process one coupling
                auto from_bag = get_engine_by_model<submodel_from, CST>(cst).outbox();
                auto& from_messages = get_messages<submodel_output_port>(from_bag);
                auto& to_messages = get_messages<external_output_port>(messages);
                to_messages.insert(to_messages.end(), from_messages.begin(), from_messages.end());

                //logging data
                std::ostringstream oss;
                logger::implode(oss, from_messages);
                std::string from_messages_str = oss.str();
                std::string from_port_str = boost::typeindex::type_id<submodel_output_port >().pretty_name();

                oss.clear();
                oss.str("");
                logger::implode(oss, to_messages);
                std::string to_messages_str = oss.str();
                std::string to_port_str = boost::typeindex::type_id<external_output_port>().pretty_name();

                std::string from_model_str = boost::typeindex::type_id<submodel_from>().pretty_name();

                LOGGER::template log<
                        cadmium::logger::logger_message_routing,
                        cadmium::logger::coor_routing_collect_eoc
                >(from_messages_str, to_messages_str, from_port_str, to_port_str, from_model_str);

                //iterate
                collect_messages_by_eoc_impl<TIME, EOC, S-1, OUT_BAG, CST, LOGGER>::fill(messages, cst);
            }
        };

        template<typename TIME, typename EOC, typename OUT_BAG, typename CST, typename LOGGER>
        struct collect_messages_by_eoc_impl<TIME, EOC, 0, OUT_BAG, CST, LOGGER>{
            static void fill(OUT_BAG& messages, CST& cst){} //nothing to do here
        };

        template<typename TIME, typename EOC, typename OUT_BAG, typename CST,typename LOGGER>
        OUT_BAG collect_messages_by_eoc(CST& cst){
            OUT_BAG ret;//if the subcoordinators active are not connected by EOC, no output is generated
            collect_messages_by_eoc_impl<TIME, EOC, std::tuple_size<EOC>::value, OUT_BAG, CST, LOGGER>::fill(ret, cst);
            return ret;
        }

        //advance the simulation in every subengine
        template <typename TIME, typename CST>
        void advance_simulation_in_subengines(const TIME& t, CST& subcoordinators) {
            auto advance_simulation = [&t](auto & c) -> void { c.advance_simulation(t); };
            cadmium::helper::for_each<CST>(subcoordinators, advance_simulation);
        }


        //route messages following ICs
        template<typename TIME, typename CST, typename ICs, std::size_t S, typename LOGGER>
        struct route_internal_coupled_messages_on_subcoordinators_impl{
            using current_IC=typename std::tuple_element<S-1, ICs>::type;
            using from_model=typename current_IC::template from_model<TIME>;
            using from_port=typename current_IC::from_model_output_port;
            using to_model=typename current_IC::template to_model<TIME>;
            using to_port=typename current_IC::to_model_input_port;

            using from_model_type=typename get_engine_type_by_model<from_model, CST>::type;
            using to_model_type=typename get_engine_type_by_model<to_model, CST>::type;
            static void route(const TIME& t, CST& engines){
                //route messages for 1 coupling
                from_model_type& from_engine = get_engine_by_model<from_model, CST>(engines);
                to_model_type& to_engine=get_engine_by_model<to_model, CST>(engines);

                //add the messages
                auto& from_messages = cadmium::get_messages<from_port>(from_engine._outbox);
                auto& to_messages = cadmium::get_messages<to_port>(to_engine._inbox);
                to_messages.insert(to_messages.end(), from_messages.begin(), from_messages.end());

                //logging data
                std::ostringstream oss;
                logger::implode(oss, from_messages);
                std::string from_messages_str = oss.str();
                std::string from_port_str = boost::typeindex::type_id<from_port>().pretty_name();
                std::string from_model_str = boost::typeindex::type_id<from_model>().pretty_name();

                oss.clear();
                oss.str("");
                logger::implode(oss, to_messages);
                std::string to_messages_str = oss.str();
                std::string to_port_str = boost::typeindex::type_id<to_port>().pretty_name();
                std::string to_model_str = boost::typeindex::type_id<to_model>().pretty_name();

                LOGGER::template log<
                        cadmium::logger::logger_message_routing,
                        cadmium::logger::coor_routing_collect_ic
                >(from_messages_str, to_messages_str, from_port_str, from_model_str, to_port_str, to_model_str);

                //iterate
                route_internal_coupled_messages_on_subcoordinators_impl<TIME, CST, ICs, S-1, LOGGER>::route(t, engines);
            }
        };

        template<typename TIME, typename CST, typename ICs, typename LOGGER>
        struct route_internal_coupled_messages_on_subcoordinators_impl<TIME, CST, ICs, 0, LOGGER>{
            static void route(const TIME& t, CST& subcoordinators){
            //nothing to do here
            }
        };

        template <typename TIME, typename CST, typename ICs, typename LOGGER >
        void route_internal_coupled_messages_on_subcoordinators(const TIME& t, CST& cst){
            route_internal_coupled_messages_on_subcoordinators_impl<TIME, CST, ICs, std::tuple_size<ICs>::value, LOGGER>::route(t, cst);
            return;
        }

        template<typename TIME, typename INBAGS, typename CST, typename EICs, size_t S, typename LOGGER>
        struct route_external_input_coupled_messages_on_subcoordinators_impl{
            
            static void route(TIME t, const INBAGS& inbox, CST& engines){
                if constexpr (S != 0) {
                    using current_EIC=typename std::tuple_element<S-1, EICs>::type;
                    using from_port=typename current_EIC::external_input_port;
                    using to_model=typename current_EIC::template submodel<TIME>;
                    using to_port=typename current_EIC::submodel_input_port;

                    auto& to_engine=get_engine_by_model<to_model, CST>(engines);
                    auto& from_messages = cadmium::get_messages<from_port>(inbox);
                    auto& to_messages = cadmium::get_messages<to_port>(to_engine._inbox);
                    to_messages.insert(to_messages.end(), from_messages.begin(), from_messages.end());

                    //logging data
                    std::ostringstream oss;
                    logger::implode(oss, from_messages);
                    std::string from_messages_str = oss.str();
                    std::string from_port_str = boost::typeindex::type_id<from_port>().pretty_name();

                    oss.clear();
                    oss.str("");
                    logger::implode(oss, to_messages);
                    std::string to_messages_str = oss.str();
                    std::string to_port_str = boost::typeindex::type_id<to_port>().pretty_name();

                    std::string to_model_str = boost::typeindex::type_id<to_model>().pretty_name();

                    LOGGER::template log<
                            cadmium::logger::logger_message_routing,
                            cadmium::logger::coor_routing_collect_eic
                    >(from_messages_str, to_messages_str, to_port_str, to_model_str, from_port_str);

                    //iterate
                    route_external_input_coupled_messages_on_subcoordinators_impl<TIME, INBAGS, CST, EICs, S-1, LOGGER>::route(t, inbox, engines);

                }
            }
        };

        template <typename TIME, typename INBAGS, typename CST, typename EICs, typename LOGGER >
        void route_external_input_coupled_messages_on_subcoordinators(const TIME& t, const INBAGS& inbox, CST& cst){
                route_external_input_coupled_messages_on_subcoordinators_impl<TIME, INBAGS, CST, EICs, std::tuple_size<EICs>::value, LOGGER>::route(t, inbox, cst);
            return;
        }

        //Checking all bag of inbox or outbox are empty
        template<class BOX>
        decltype(auto) all_bags_empty(BOX const& box) {
            auto check_empty = [](auto const&... b)->decltype(auto) {
                return (b.messages.empty() && ...);
            };
            return std::apply(check_empty, box);
        }
    }

}
#endif // PDEVS_ENGINE_HELPERS_HPP
