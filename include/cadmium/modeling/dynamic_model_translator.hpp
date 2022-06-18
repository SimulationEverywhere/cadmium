/**
 * Copyright (c) 2018, Laouen M. L. Belloli
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

#ifndef CADMIUM_DYNAMIC_MODEL_TRANSLATOR_HPP
#define CADMIUM_DYNAMIC_MODEL_TRANSLATOR_HPP

#include <utility>
#include <map>
#include <memory>

#include <cadmium/engine/pdevs_dynamic_link.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_atomic.hpp>
#include <cadmium/modeling/dynamic_asynchronus_atomic.hpp>
#include <cadmium/modeling/dynamic_coupled.hpp>

namespace cadmium {
    namespace dynamic {
        namespace translate {

            using models_by_type = typename std::map<std::type_index, std::shared_ptr<cadmium::dynamic::modeling::model>>;

            template<typename PORT_FROM, typename PORT_TO>
            std::shared_ptr<cadmium::dynamic::engine::link_abstract> make_link() {
                std::shared_ptr<cadmium::dynamic::engine::link_abstract> spLink = std::make_shared<cadmium::dynamic::engine::link<PORT_FROM, PORT_TO>>();
                return spLink;
            }

            template<typename PORTS>
            cadmium::dynamic::modeling::Ports make_ports() {
                cadmium::dynamic::modeling::Ports ret;
                auto add_port = [&ret] (auto &p) -> void {
                    std::type_index port_type_index = typeid(p);
                    ret.push_back(port_type_index);
                };
                PORTS ports;
                cadmium::helper::for_each<PORTS>(ports, add_port);
                return ret;
            }

            template<typename TIME, typename EIC_TUPLE, size_t S>
            struct make_dynamic_ic_impl{
                using current_IC=typename std::tuple_element<S-1, EIC_TUPLE>::type;

                template<typename T>
                using from_model=typename current_IC::template from_model<T>;
                using from_port=typename current_IC::from_model_output_port;
                template<typename T>
                using to_model=typename current_IC::template to_model<T>;
                using to_port=typename current_IC::to_model_input_port;

                static void value(models_by_type& translated_models, cadmium::dynamic::modeling::ICs& ret) {

                    std::type_index to_model_type(typeid(to_model<TIME>));
                    if (translated_models.find(to_model_type) == translated_models.cend()) {
                        throw std::domain_error("EIC destination  " + boost::typeindex::type_id<to_model<TIME>>().pretty_name() + " is not in the coupled sub models list");
                    }
                    std::string to_id = translated_models.at(to_model_type)->get_id();

                    std::type_index from_model_type(typeid(from_model<TIME>));
                    if (translated_models.find(from_model_type) == translated_models.cend()) {
                        throw std::domain_error("EIC destination model " + boost::typeindex::type_id<from_model<TIME>>().pretty_name() + " is not in the coupled sub models list");
                    }
                    std::string from_id = translated_models.at(from_model_type)->get_id();

                    std::shared_ptr<cadmium::dynamic::engine::link_abstract> new_link = cadmium::dynamic::translate::make_link<from_port, to_port>();
                    ret.emplace_back(from_id, to_id, new_link);

                    //iterate
                    make_dynamic_ic_impl<TIME, EIC_TUPLE, S-1>::value(translated_models, ret);
                }
            };

            template<typename TIME, typename IC_TUPLE>
            struct make_dynamic_ic_impl<TIME, IC_TUPLE, 0>{
                static void value(models_by_type& translated_models, cadmium::dynamic::modeling::ICs& ret){
                    //do nothing
                }
            };

            /**
             * @brief Constructs the correct cadmium::dynamic::modeling::ICs object from the original cadmium std::tuple<IC..> type
             *
             * @note The original cadmium IC types must contain models with default constructor and with the method get_id()
             * for this purpose coupled models can be made as a derived class from cadmium::dynamic::modeling::coupled with the constructor
             * set with a custom id for coupled models and for the atomic models the cadmium::dynamic::modeling::atomic models does have the
             * get_id() constructor.
             *
             * @tparam TIME - the model time type
             * @tparam IC_TUPLE - the type of the original cadmium IC tuple.
             * @return The constructed cadmium::dynamic::modeling::ICs object
             */
            template <typename TIME, typename IC_TUPLE>
            cadmium::dynamic::modeling::ICs make_dynamic_ic(models_by_type& translated_models){
                cadmium::dynamic::modeling::ICs ret;
                make_dynamic_ic_impl<TIME, IC_TUPLE, std::tuple_size<IC_TUPLE>::value>::value(translated_models, ret);
                return ret;
            }

            template<typename TIME, typename EIC_TUPLE, size_t S>
            struct make_dynamic_eic_impl{
                using current_EIC=typename std::tuple_element<S-1, EIC_TUPLE>::type;
                using from_port=typename current_EIC::external_input_port;
                template <typename T>
                using to_model=typename current_EIC::template submodel<T>;
                using to_port=typename current_EIC::submodel_input_port;

                static void value(models_by_type& translated_models, cadmium::dynamic::modeling::EICs& ret) {

                    std::type_index model_type(typeid(to_model<TIME>));
                    if (translated_models.find(model_type) == translated_models.cend()) {
                        throw std::domain_error("EIC destination model " + boost::typeindex::type_id<to_model<TIME>>().pretty_name() + " is not in the coupled sub models list");
                    }
                    std::string to_id = translated_models.at(model_type)->get_id();

                    std::shared_ptr<cadmium::dynamic::engine::link_abstract> new_link = cadmium::dynamic::translate::make_link<from_port, to_port>();
                    ret.emplace_back(to_id, new_link);

                    //iterate
                    make_dynamic_eic_impl<TIME, EIC_TUPLE, S-1>::value(translated_models, ret);
                }
            };

            template<typename TIME, typename EIC_TUPLE>
            struct make_dynamic_eic_impl<TIME, EIC_TUPLE, 0>{
                static void value(models_by_type& translated_models, cadmium::dynamic::modeling::EICs& ret){
                    //do nothing
                }
            };

            /**
             * @brief Constructs the correct cadmium::dynamic::modeling::EICs object from the original cadmium std::tuple<EIC..> type
             *
             * @note The original cadmium EIC types must contain a model with default constructor and with the method get_id()
             * for this purpose coupled models can be made as a derived class from cadmium::dynamic::modeling::coupled with the constructor
             * set with a custom id for coupled models and for the atomic models the cadmium::dynamic::modeling::atomic models does have the
             * get_id() constructor.
             *
             * @tparam TIME - the model time type
             * @tparam EIC_TUPLE - the type of the original cadmium EIC tuple.
             * @return The constructed cadmium::dynamic::modeling::EICs object
             */
            template <typename TIME, typename EIC_TUPLE>
            cadmium::dynamic::modeling::EICs make_dynamic_eic(models_by_type& translated_models){
                cadmium::dynamic::modeling::EICs ret;
                make_dynamic_eic_impl<TIME, EIC_TUPLE, std::tuple_size<EIC_TUPLE>::value>::value(translated_models, ret);
                return ret;
            }

            template<typename TIME, typename EOC_TUPLE, size_t S>
            struct make_dynamic_eoc_impl{
                using current_EIC=typename std::tuple_element<S-1, EOC_TUPLE>::type;
                using from_port=typename current_EIC::submodel_output_port;
                template<typename T>
                using from_model=typename current_EIC::template submodel<T>;
                using to_port=typename current_EIC::external_output_port;

                static void value(const models_by_type& translated_models, cadmium::dynamic::modeling::EOCs& ret) {

                    std::type_index model_type(typeid(from_model<TIME>));
                    if (translated_models.find(model_type) == translated_models.cend()) {
                        throw std::domain_error("EIC destination model " + boost::typeindex::type_id<from_model<TIME>>().pretty_name() + " is not in the coupled sub models list");
                    }
                    std::string from_id = translated_models.at(model_type)->get_id();

                    std::shared_ptr<cadmium::dynamic::engine::link_abstract> new_link = cadmium::dynamic::translate::make_link<from_port, to_port>();
                    ret.emplace_back(from_id, new_link);

                    //iterate
                    make_dynamic_eoc_impl<TIME, EOC_TUPLE, S-1>::value(translated_models, ret);
                }
            };

            template<typename TIME, typename EOC_TUPLE>
            struct make_dynamic_eoc_impl<TIME, EOC_TUPLE, 0> {
                static void value(const models_by_type& translated_models, cadmium::dynamic::modeling::EOCs& ret){
                    //do nothing
                }
            };

            /**
             * @brief Constructs the correct cadmium::dynamic::modeling::EOCs object from the original cadmium std::tuple<EOC..> type
             *
             * @note The original cadmium EOC types must contain a model with default constructor and with the method get_id()
             * for this purpose coupled models can be made as a derived class from cadmium::dynamic::modeling::coupled with the constructor
             * set with a custom id for coupled models and for the atomic models the cadmium::dynamic::modeling::atomic models does have the
             * get_id() constructor.
             *
             * @tparam EOC_TUPLE - the type of the original cadmium EOC tuple.
             * @tparam TIME - the model time type
             * @return The constructed cadmium::dynamic::modeling::EOCs object
             */
            template <class TIME, typename EOC_TUPLE>
            cadmium::dynamic::modeling::EOCs make_dynamic_eoc(const models_by_type& translated_models) {
                cadmium::dynamic::modeling::EOCs ret;
                make_dynamic_eoc_impl<TIME, EOC_TUPLE, std::tuple_size<EOC_TUPLE>::value>::value(translated_models, ret);
                return ret;
            }

            template<typename PORT_FROM, typename PORT_TO>
            cadmium::dynamic::modeling::EOC make_EOC(std::string model_from) {
                std::shared_ptr<cadmium::dynamic::engine::link_abstract> eoc_link = cadmium::dynamic::translate::make_link<PORT_FROM, PORT_TO>();
                return cadmium::dynamic::modeling::EOC(model_from, eoc_link);
            }

            template<typename PORT_FROM, typename PORT_TO>
            cadmium::dynamic::modeling::EIC make_EIC(std::string model_to) {
                std::shared_ptr<cadmium::dynamic::engine::link_abstract> eic_link = cadmium::dynamic::translate::make_link<PORT_FROM, PORT_TO>();
                return cadmium::dynamic::modeling::EIC(model_to, eic_link);
            }

            template<typename PORT_FROM, typename PORT_TO>
            cadmium::dynamic::modeling::IC make_IC(std::string model_from, std::string model_to) {
                std::shared_ptr<cadmium::dynamic::engine::link_abstract> ic_link = cadmium::dynamic::translate::make_link<PORT_FROM, PORT_TO>();
                return cadmium::dynamic::modeling::IC(model_from, model_to, ic_link);
            }

            /**
             * @brief creates a cadmium::dynamic::modeling::atomic<ATOMIC, TIME> model and returns
             * a shared pointer to it absctract base class cadmium::dynamic::atomic_abstract<TIME>
             *
             * @tparam ATOMIC - The atomic model type.
             * @tparam TIME - The time type to which the model must be instanciated.
             */
            template <template<typename T> class ATOMIC, typename TIME>
            struct make_dynamic_atomic_model_impl {

                std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> make() {
                    std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> sp_model = std::make_shared<cadmium::dynamic::modeling::atomic<ATOMIC, TIME>>();
                    return sp_model;
                }
            };

            template <template<typename T> class ATOMIC, typename TIME>
            std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> make_dynamic_atomic_model() {
                return make_dynamic_atomic_model_impl<ATOMIC, TIME>().make();
            }

            template< template <typename T> typename ATOMIC, typename TIME, typename... Args >
            std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> make_dynamic_atomic_model(const std::string& model_id, Args&&... args) {
                std::shared_ptr<cadmium::dynamic::modeling::atomic_abstract<TIME>> sp_model = std::make_shared<cadmium::dynamic::modeling::atomic<ATOMIC, TIME, Args...>>(model_id, std::forward<Args>(args)...);
                return sp_model;
            }
            /*******************************************************************************************************/
            /*  */
            template <template<typename T> class ATOMIC, typename TIME>
            struct make_dynamic_asynchronus_atomic_model_impl {

                std::shared_ptr<cadmium::dynamic::modeling::asynchronus_atomic_abstract<TIME>> make() {
                    std::shared_ptr<cadmium::dynamic::modeling::asynchronus_atomic_abstract<TIME>> sp_model = std::make_shared<cadmium::dynamic::modeling::asynchronus_atomic<ATOMIC, TIME>>();
                    return sp_model;
                }
            };

            template <template<typename T> class ATOMIC, typename TIME>
            std::shared_ptr<cadmium::dynamic::modeling::asynchronus_atomic_abstract<TIME>> make_dynamic_asynchronus_atomic_model() {
                return make_dynamic_atomic_model_impl<ATOMIC, TIME>().make();
            }

            template< template <typename T> typename ATOMIC, typename TIME, typename... Args >
            std::shared_ptr<cadmium::dynamic::modeling::asynchronus_atomic_abstract<TIME>> make_dynamic_asynchronus_atomic_model(const std::string& model_id, Args&&... args) {
                std::shared_ptr<cadmium::dynamic::modeling::asynchronus_atomic_abstract<TIME>> sp_model = std::make_shared<cadmium::dynamic::modeling::asynchronus_atomic<ATOMIC, TIME, Args...>>(model_id, std::forward<Args>(args)...);
                return sp_model;
            }

            /*******************************************************************************************************/


            template<typename TIME, template<typename T> class MT, template<template<typename T2> class M> class COUPLED_TRANSLATOR, size_t S>
            struct make_dynamic_models_impl{
                template<typename P>
                using current = typename std::tuple_element<S - 1, MT<P>>::type;
                using current_translator = typename std::conditional<cadmium::concept::is_atomic<current>::value(), make_dynamic_atomic_model_impl<current, TIME>, COUPLED_TRANSLATOR<current>>::type;

                static void make_models(models_by_type &ret) {

                    current_translator translator;
                    std::shared_ptr<cadmium::dynamic::modeling::model> sp_current = translator.make();
                    ret.emplace(typeid(current<TIME>), sp_current);

                    make_dynamic_models_impl<TIME, MT, COUPLED_TRANSLATOR, S - 1>::make_models(ret);
                }
            };

            template<typename TIME, template<typename T> class MT, template<template<typename T2> class M> class COUPLED_TRANSLATOR>
            struct make_dynamic_models_impl<TIME, MT, COUPLED_TRANSLATOR, 0>{

                static void make_models(models_by_type &ret) {
                    // nothing to do here;
                }
            };

            template <typename TIME, template<typename T> class MT, template<template<typename T2> class M> class COUPLED_TRANSLATOR>
            models_by_type make_dynamic_models() {
                models_by_type ret;
                make_dynamic_models_impl<TIME, MT, COUPLED_TRANSLATOR, std::tuple_size<MT<TIME>>::value>::make_models(ret);
                return ret;
            }

            template <typename TIME, template<typename T1> class MODEL>
            struct make_dynamic_coupled_model_impl {

                template<template<typename T2> class M>
                using coupled_model_translator = make_dynamic_coupled_model_impl<TIME, M>;

                //types for subcoordination
                template<typename T3>
                using submodels_t = typename MODEL<TIME>::template models<T3>;
                using input_ports_t = typename MODEL<TIME>::input_ports;
                using output_ports_t = typename MODEL<TIME>::output_ports;
                using eoc_t = typename MODEL<TIME>::external_output_couplings;
                using eic_t = typename MODEL<TIME>::external_input_couplings;
                using ic_t = typename MODEL<TIME>::internal_couplings;

                std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> make() {

                    models_by_type sub_models = make_dynamic_models<TIME, submodels_t, coupled_model_translator>();

                    std::string model_id = boost::typeindex::type_id<MODEL<TIME>>().pretty_name();
                    cadmium::dynamic::modeling::Ports input_ports = cadmium::dynamic::translate::make_ports<input_ports_t>();
                    cadmium::dynamic::modeling::Ports output_ports = cadmium::dynamic::translate::make_ports<output_ports_t>();
                    cadmium::dynamic::modeling::EOCs eocs = cadmium::dynamic::translate::make_dynamic_eoc<TIME, eoc_t>(sub_models);
                    cadmium::dynamic::modeling::EICs eics = cadmium::dynamic::translate::make_dynamic_eic<TIME, eic_t>(sub_models);
                    cadmium::dynamic::modeling::ICs ics = cadmium::dynamic::translate::make_dynamic_ic<TIME, ic_t>(sub_models);

                    cadmium::dynamic::modeling::Models models;
                    for (auto& m : sub_models) {
                        models.push_back(m.second);
                    }

                    std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> coupled = std::make_shared<cadmium::dynamic::modeling::coupled<TIME>>(
                            model_id,
                            models,
                            input_ports,
                            output_ports,
                            eics,
                            eocs,
                            ics
                    );

                    return coupled;
                }
            };

            template<typename TIME,  template<typename T> class MODEL>
            std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> make_dynamic_coupled_model() {
                return make_dynamic_coupled_model_impl<TIME, MODEL>().make();
            }
        }
    }
}

#endif //CADMIUM_DYNAMIC_MODEL_TRANSLATOR_HPP
