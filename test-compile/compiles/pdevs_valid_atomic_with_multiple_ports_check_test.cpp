/**
 * Copyright (c) 2013-2016, Damian Vicino
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
 * Test that a valid atomic model with multiple ports does not stop compilation on atomic_model_assert.
 */

#include<cadmium/modeling/ports.hpp>
#include<cadmium/concept/atomic_model_assert.hpp>
#include<tuple>
#include<cadmium/modeling/message_bag.hpp>


/**
 * This model has no logic, only used for structural validation tests
 */
template<typename TIME>
struct valid_atomic_with_multiple_ports
{
    struct in_one : public cadmium::in_port<int>{};
    struct in_two : public cadmium::in_port<float>{};
    struct in_three: public cadmium::in_port<int>{};
    struct out_one : public cadmium::out_port<int[]>{};
    struct out_two : public cadmium::out_port<float>{};
    struct out_three : public cadmium::out_port<int[]>{};

    constexpr valid_atomic_with_multiple_ports() noexcept {}
    using state_type=int;
    state_type state=0;
    using input_ports=std::tuple<in_one, in_two, in_three>;
    using output_ports=std::tuple<out_one, out_two, out_three>;

    void internal_transition(){}
    void external_transition(TIME e, typename cadmium::make_message_bags<input_ports>::type mbs){}
    void confluence_transition(TIME e, typename cadmium::make_message_bags<input_ports>::type mbs){}
    typename cadmium::make_message_bags<output_ports>::type output() const{}
    TIME time_advance() const{}
};

int main(){
    cadmium::concept::pdevs::atomic_model_assert<valid_atomic_with_multiple_ports>();
}