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
#include <cadmium/modeling/dynamic_models_helpers.hpp>
#include <cadmium/modeling/dynamic_atomic.hpp>

namespace cadmium {
    namespace dynamic {
        namespace translate {

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
                using from_model=typename current_IC::template from_model<TIME>;
                using from_port=typename current_IC::from_model_output_port;
                using to_model=typename current_IC::template to_model<TIME>;
                using to_port=typename current_IC::to_model_input_port;

                static void value(cadmium::dynamic::modeling::EICs& ret) {

                    to_model sp_to_model = to_model();
                    std::string to_id = sp_to_model.get_id();

                    from_model sp_from_model = from_model();
                    std::string from_id = sp_from_model.get_id();

                    std::shared_ptr<cadmium::dynamic::link_abstract> new_link = cadmium::dynamic::make_link<from_port, to_port>();
                    ret.emplace_back(to_id, from_id, new_link);

                    //iterate
                    make_dynamic_ic_impl<TIME, EIC_TUPLE, S-1>::value(ret);
                }
            };

            template<typename TIME, typename IC_TUPLE>
            struct make_dynamic_ic_impl<TIME, IC_TUPLE, 0>{
                static void value(cadmium::dynamic::modeling::ICs& ret){
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
            cadmium::dynamic::modeling::ICs make_dynamic_ic(){
                cadmium::dynamic::modeling::ICs ret;
                make_dynamic_ic_impl<TIME, IC_TUPLE, std::tuple_size<IC_TUPLE>::value>::value(ret);
                return ret;
            }

            template<typename TIME, typename EIC_TUPLE, size_t S>
            struct make_dynamic_eic_impl{
                using current_EIC=typename std::tuple_element<S-1, EIC_TUPLE>::type;
                using from_port=typename current_EIC::external_input_port;
                using to_model=typename current_EIC::template submodel<TIME>;
                using to_port=typename current_EIC::submodel_input_port;

                static void value(cadmium::dynamic::modeling::EICs& ret) {

                    to_model sp_model = to_model();
                    std::string to_id = sp_model.get_id();

                    std::shared_ptr<cadmium::dynamic::link_abstract> new_link = cadmium::dynamic::make_link<from_port, to_port>();
                    ret.emplace_back(to_id, new_link);

                    //iterate
                    make_dynamic_eic_impl<TIME, EIC_TUPLE, S-1>::value(ret);
                }
            };

            template<typename TIME, typename EIC_TUPLE>
            struct make_dynamic_eic_impl<TIME, EIC_TUPLE, 0>{
                static void value(cadmium::dynamic::modeling::EICs& ret){
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
            cadmium::dynamic::modeling::EICs make_dynamic_eic(){
                cadmium::dynamic::modeling::EICs ret;
                make_dynamic_eic_impl<TIME, EIC_TUPLE, std::tuple_size<EIC_TUPLE>::value>::value(ret);
                return ret;
            }

            template<typename TIME, typename EOC_TUPLE, size_t S>
            struct make_dynamic_eoc_impl{
                using current_EIC=typename std::tuple_element<S-1, EOC_TUPLE>::type;
                using from_port=typename current_EIC::submodel_output_port;
                template<typename T>
                using from_model=typename current_EIC::template submodel<T>;
                using to_port=typename current_EIC::external_output_port;

                static void value(cadmium::dynamic::modeling::EOCs& ret) {

                    std::string from_id = from_model<TIME>().get_id();

                    std::shared_ptr<cadmium::dynamic::link_abstract> new_link = cadmium::dynamic::make_link<from_port, to_port>();
                    ret.emplace_back(from_id, new_link);

                    //iterate
                    make_dynamic_eoc_impl<TIME, EOC_TUPLE, S-1>::value(ret);
                }
            };

            template<typename TIME, typename EOC_TUPLE>
            struct make_dynamic_eoc_impl<TIME, EOC_TUPLE, 0> {
                static void value(cadmium::dynamic::modeling::EOCs& ret){
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
            template <typename TIME, typename EOC_TUPLE>
            cadmium::dynamic::modeling::EOCs make_dynamic_eoc() {
                cadmium::dynamic::modeling::EOCs ret;
                make_dynamic_eoc_impl<TIME, EOC_TUPLE, std::tuple_size<EOC_TUPLE>::value>::value(ret);
                return ret;
            }
        }
    }
}

#endif //CADMIUM_DYNAMIC_MODEL_TRANSLATOR_HPP
