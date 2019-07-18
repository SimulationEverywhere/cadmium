/**
 * Copyright (c) 2013-2019, Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis
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

/**
 * Test that an atomic model with no input_ports transition fails compilation on atomic_model_assert
 * This is different to say that it has an empty tuple of ports, which is a valid model definition
 */

#include<cadmium/modeling/ports.hpp>
#include<cadmium/concept/atomic_model_assert.hpp>
#include<tuple>
#include<cadmium/modeling/message_bag.hpp>


/**
 * This model has no logic, only used for structural validation tests.
 * In this case it is missing the declaration of input_ports type
 * For external and confluence transition fuctions defined input as empty tuple of ports
 */
template<typename TIME>
struct pdevs_atomic_model_with_no_input_ports {
    struct out : public cadmium::out_port<int> {
    };

    constexpr pdevs_atomic_model_with_no_input_ports() noexcept {}

    using state_type=int;
    state_type state = 0;
    using output_ports=std::tuple<out>;

    void internal_transition() {}

    void external_transition(TIME e, typename cadmium::make_message_bags<std::tuple<>>::type mbs) {}

    void confluence_transition(TIME e, typename cadmium::make_message_bags<std::tuple<>>::type mbs) {}

    typename cadmium::make_message_bags<output_ports>::type output() const {}

    TIME time_advance() const {}
};

int main() {
    cadmium::concept::pdevs::atomic_model_assert<pdevs_atomic_model_with_no_input_ports>();
}