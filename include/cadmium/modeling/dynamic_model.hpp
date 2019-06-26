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

#include <iostream>
#include <vector>
#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/engine/pdevs_dynamic_link.hpp>

namespace cadmium {
    namespace dynamic {
        namespace modeling {

            struct EOC {
                std::string _from;
                std::shared_ptr<cadmium::dynamic::engine::link_abstract> _link;

                EOC() = delete;

                EOC(std::string from, std::shared_ptr<cadmium::dynamic::engine::link_abstract> l)
                        : _from(from), _link(l) {}

                EOC(const EOC& other)
                        : _from(other._from), _link(other._link) {}
            };

            struct EIC {
                std::string _to;
                std::shared_ptr<cadmium::dynamic::engine::link_abstract> _link;

                EIC() = delete;

                EIC(std::string to, std::shared_ptr<cadmium::dynamic::engine::link_abstract> l)
                        : _to(to), _link(l) {}

                EIC(const EIC& other)
                        : _to(other._to), _link(other._link) {}
            };

            struct IC {
                std::string _from;
                std::string _to;
                std::shared_ptr<cadmium::dynamic::engine::link_abstract> _link;

                IC() = delete;

                IC(std::string from, std::string to, std::shared_ptr<cadmium::dynamic::engine::link_abstract> l)
                        : _from(from), _to(to), _link(l) {}

                IC(const IC& other)
                        : _from(other._from), _to(other._to), _link(other._link) {}
            };

            using Ports = std::vector<std::type_index>;
            using EICs = std::vector<EIC>;
            using EOCs = std::vector<EOC>;
            using ICs = std::vector<IC>;

            using initilizer_list_Ports = std::initializer_list<std::type_index>;
            using initializer_list_EOCs = std::initializer_list<EOC>;
            using initializer_list_EICs = std::initializer_list<EIC>;
            using initializer_list_ICs = std::initializer_list<IC>;

            /**
             * @brief Empty class to allow pointer based polymorphism between classes derived from
             * atomic and coupled models.
             */
            class model {
            public:
                virtual std::string get_id() const = 0;
                virtual cadmium::dynamic::modeling::Ports get_input_ports() const = 0;
                virtual cadmium::dynamic::modeling::Ports get_output_ports() const = 0;
            };

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
            class atomic_abstract : public cadmium::dynamic::modeling::model {
            public:
                // Simulation purpose, because the model type are hidden, we need the model wrapper
                // help for dealing with the model type dependant methods for message routing.
                virtual std::string get_id() const override = 0;
                virtual cadmium::dynamic::modeling::Ports get_input_ports() const override = 0;
                virtual cadmium::dynamic::modeling::Ports get_output_ports() const override = 0;

                // Logging purpose methods, also because the model type is needed for logging the
                // state and message bags.
                virtual std::string model_state_as_string() const = 0;
                virtual std::string messages_by_port_as_string(cadmium::dynamic::message_bags outbox) const = 0;

                // atomic model methods
                virtual void internal_transition() = 0;
                virtual void external_transition(TIME e, cadmium::dynamic::message_bags dynamic_bags) = 0;
                virtual void confluence_transition(TIME e, cadmium::dynamic::message_bags dynamic_bags) = 0;
                virtual dynamic::message_bags output() const = 0;
                virtual TIME time_advance() const = 0;
            };

            class AsyncEventSubject {
                std::vector <class AsyncEventObserver *> views;
                std::string _id;
            public:
                AsyncEventSubject(std::string subId){
                    _id = subId;
                }
                void attach(AsyncEventObserver *obs) {
                    views.push_back(obs);
                }
                std::string getId() {
                    return _id;
                }
                void notify();
            };
            
            class AsyncEventObserver {
                std::vector < class AsyncEventSubject * > sub;
                
            public:
                AsyncEventObserver(AsyncEventSubject *s) {
                    s->attach(this);
                    sub.push_back(s);
                }
                
                AsyncEventObserver(std::vector < class AsyncEventSubject * > s) {
                    sub = s;
                    for (unsigned int i = 0; i < sub.size(); i++)
                        sub[i]->attach(this);
                }

                virtual void update() = 0;

            protected:
                bool interrupted;
                std::vector <class AsyncEventSubject *> getSubject() {
                    return sub;
                }
            };
            
            void AsyncEventSubject::notify() {
                for (unsigned int i = 0; i < views.size(); i++)
                    views[i]->update();
            }

            template<typename TIME>
            class asynchronus_atomic_abstract : public cadmium::dynamic::modeling::model, public virtual cadmium::dynamic::modeling::AsyncEventSubject {
            public:
                // Simulation purpose, because the model type are hidden, we need the model wrapper
                // help for dealing with the model type dependant methods for message routing.
                virtual std::string get_id() const override = 0;
                virtual cadmium::dynamic::modeling::Ports get_input_ports() const override = 0;
                virtual cadmium::dynamic::modeling::Ports get_output_ports() const override = 0;

                // Logging purpose methods, also because the model type is needed for logging the
                // state and message bags.
                virtual std::string model_state_as_string() const = 0;
                virtual std::string messages_by_port_as_string(cadmium::dynamic::message_bags outbox) const = 0;

                // atomic model methods
                virtual void internal_transition() = 0;
                virtual void external_transition(TIME e, cadmium::dynamic::message_bags dynamic_bags) = 0;
                virtual void confluence_transition(TIME e, cadmium::dynamic::message_bags dynamic_bags) = 0;
                virtual dynamic::message_bags output() const = 0;
                virtual TIME time_advance() const = 0;
            };

            using Models = std::vector<std::shared_ptr<cadmium::dynamic::modeling::model>>;
            using initializer_list_Models = std::initializer_list<std::shared_ptr<cadmium::dynamic::modeling::model>>;
        }
    }
}

#endif // CADMIUM_ATOMIC_HPP
