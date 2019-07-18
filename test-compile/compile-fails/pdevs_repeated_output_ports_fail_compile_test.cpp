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
 * Test that a valid atomic model does not stop compilation on atomic_model_assert.
 */

#include<cadmium/modeling/ports.hpp>
#include<cadmium/concept/atomic_model_assert.hpp>
#include<tuple>
#include<cadmium/modeling/message_bag.hpp>


/**
* Test that when an atomic model has duplicated output ports, atomic_model_assert fails compilation */
template<typename TIME>
struct devs_atomic_model_with_repeated_output_ports {
    struct in : public cadmium::in_port<int> {
    };

    struct out_one : public cadmium::out_port<int> {
    };
    struct out_two : public cadmium::out_port<int> {
    };

    constexpr devs_atomic_model_with_repeated_output_ports() noexcept {}

    using state_type=int;
    state_type state = 0;
    using input_ports=std::tuple<in>;
    using output_ports=std::tuple<out_one, out_two, out_one>;

    void internal_transition() {}

    void external_transition(TIME e, typename cadmium::make_message_bags<input_ports>::type mbs) {}

    void confluence_transition(TIME e, typename cadmium::make_message_bags<input_ports>::type mbs) {}

    typename cadmium::make_message_bags<output_ports>::type output() const {}

    TIME time_advance() const {}
};

int main() {
    cadmium::concept::pdevs::atomic_model_assert<devs_atomic_model_with_repeated_output_ports>();
}