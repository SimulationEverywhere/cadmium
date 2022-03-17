/**
 * Copyright (c) 2022, Guillermo Trabes
 * Carleton University, Universidad Nacional de San Luis
 *
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

#ifndef CADMIUM_PDEVS_DYNAMIC_IMPROVED_NAIVE_PARALLEL_ENGINE_HELPERS_HPP
#define CADMIUM_PDEVS_DYNAMIC_IMPROVED_NAIVE_PARALLEL_ENGINE_HELPERS_HPP

#include <cadmium/modeling/dynamic_message_bag.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include <cadmium/engine/pdevs_dynamic_link.hpp>
#include <cadmium/hpc_engine/improved_naive_parallel/pdevs_dynamic_improved_naive_parallel_engine.hpp>
#include <cadmium/hpc_engine/improved_naive_parallel/pdevs_dynamic_improved_naive_parallel_simulator.hpp>

namespace cadmium {
    namespace dynamic {
        namespace hpc_engine {
            namespace improved_naive_parallel {

                template<typename TIME, typename LOGGER>
                using subcoordinators_type = typename std::vector<std::shared_ptr<cadmium::dynamic::hpc_engine::improved_naive_parallel::improved_naive_parallel_simulator<TIME, LOGGER>>>;
                using external_port_couplings = typename std::map<std::string, std::vector<std::shared_ptr<cadmium::dynamic::engine::link_abstract>>>;

                template<typename TIME, typename LOGGER>
                using external_couplings = typename std::vector<
                        std::pair<
                                std::shared_ptr<cadmium::dynamic::hpc_engine::improved_naive_parallel::improved_naive_parallel_simulator<TIME, LOGGER>>,
                                std::vector<std::shared_ptr<cadmium::dynamic::engine::link_abstract>>
                        >
                >;

                template<typename TIME, typename LOGGER>
                using internal_coupling = std::pair<
                        std::pair<
                                std::shared_ptr<cadmium::dynamic::hpc_engine::improved_naive_parallel::improved_naive_parallel_simulator<TIME, LOGGER>>, // from model
                                std::shared_ptr<cadmium::dynamic::hpc_engine::improved_naive_parallel::improved_naive_parallel_simulator<TIME, LOGGER>> // to model
                        >,
                        std::vector<std::shared_ptr<cadmium::dynamic::engine::link_abstract>>
                >;

                template<typename TIME, typename LOGGER>
                using internal_couplings = typename std::vector<internal_coupling<TIME, LOGGER>>;

                template<typename TIME, typename LOGGER>
                using external_coupling = std::pair<
                        std::shared_ptr<cadmium::dynamic::hpc_engine::improved_naive_parallel::improved_naive_parallel_simulator<TIME, LOGGER>>,
                        std::vector<std::shared_ptr<cadmium::dynamic::engine::link_abstract>>
				>;

            }
        }
    }
}

#endif //CADMIUM_PDEVS_DYNAMIC_ENGINE_HELPERS_HPP
