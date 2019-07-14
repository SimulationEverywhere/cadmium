/**
 * Copyright (c) 2017-2019, Damian Vicino
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

//counting until 5 and output every 5 seconds.

#include <iostream>
#include <chrono>
#include <algorithm>
#include <cadmium/logger/tuple_to_ostream.hpp>
#include <cadmium/modeling/coupling.hpp>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/concept/coupled_model_assert.hpp>
#include <cadmium/engine/pdevs_runner.hpp>
#include <cadmium/basic_model/pdevs/accumulator.hpp>
#include <cadmium/basic_model/pdevs/int_generator_one_sec.hpp>
#include <cadmium/basic_model/pdevs/reset_generator_five_sec.hpp>
#include <cadmium/logger/common_loggers.hpp>

using namespace std;

using hclock=chrono::high_resolution_clock;

/**
 * This example uses 2 generatos ticking one every second and one every 5 seconds
 * they are grouped into a coupled model, in another coupled model we have an infinite counter.
 * The one sec generator ticks on the counter add, and the 5 seconds one in the reset of the counter.
 * Every five sec an output of the count (5) is expected.
 * This example shows simple coupled models collaborating.
 *
 * The experiment runtime is measured using the chrono library.
 */


template<typename TIME>
using test_accumulator=cadmium::basic_models::pdevs::accumulator<int, TIME>;
using test_accumulator_defs=cadmium::basic_models::pdevs::accumulator_defs<int>;
using reset_tick=cadmium::basic_models::pdevs::accumulator_defs<int>::reset_tick;

using empty_iports = std::tuple<>;
using empty_eic=std::tuple<>;
using empty_ic=std::tuple<>;

//2 generators doing output in 2 ports
using generators_oports=std::tuple<cadmium::basic_models::pdevs::int_generator_one_sec_defs::out, cadmium::basic_models::pdevs::reset_generator_five_sec_defs::out>;
using generators_submodels=cadmium::modeling::models_tuple<cadmium::basic_models::pdevs::reset_generator_five_sec, cadmium::basic_models::pdevs::int_generator_one_sec>;
using generators_eoc=std::tuple<
        cadmium::modeling::EOC<cadmium::basic_models::pdevs::reset_generator_five_sec, cadmium::basic_models::pdevs::reset_generator_five_sec_defs::out, cadmium::basic_models::pdevs::reset_generator_five_sec_defs::out>,
        cadmium::modeling::EOC<cadmium::basic_models::pdevs::int_generator_one_sec, cadmium::basic_models::pdevs::int_generator_one_sec_defs::out, cadmium::basic_models::pdevs::int_generator_one_sec_defs::out>
>;

template<typename TIME>
using coupled_generators_model=cadmium::modeling::pdevs::coupled_model<TIME, empty_iports, generators_oports, generators_submodels, empty_eic, generators_eoc, empty_ic>;

//1 accumulator wrapped in a coupled model
using accumulator_eic=std::tuple<
        cadmium::modeling::EIC<test_accumulator_defs::add, test_accumulator, test_accumulator_defs::add>,
        cadmium::modeling::EIC<test_accumulator_defs::reset, test_accumulator, test_accumulator_defs::reset>
>;
using accumulator_eoc=std::tuple<
        cadmium::modeling::EOC<test_accumulator, test_accumulator_defs::sum, test_accumulator_defs::sum>
>;

using accumulator_submodels=cadmium::modeling::models_tuple<test_accumulator>;

template<typename TIME>
using coupled_accumulator_model=cadmium::modeling::pdevs::coupled_model<TIME, typename test_accumulator<TIME>::input_ports, typename test_accumulator<TIME>::output_ports, accumulator_submodels, accumulator_eic, accumulator_eoc, empty_ic>;


//top model interconnecting the 2 coupled models

using top_outport = test_accumulator_defs::sum;
using top_oports = std::tuple<top_outport>;
using top_submodels=cadmium::modeling::models_tuple<coupled_generators_model, coupled_accumulator_model>;

using top_eoc=std::tuple<
        cadmium::modeling::EOC<coupled_accumulator_model, test_accumulator_defs::sum, top_outport>
>;
using top_ic=std::tuple<
        cadmium::modeling::IC<coupled_generators_model, cadmium::basic_models::pdevs::int_generator_one_sec_defs::out, coupled_accumulator_model, test_accumulator_defs::add>,
        cadmium::modeling::IC<coupled_generators_model, cadmium::basic_models::pdevs::reset_generator_five_sec_defs::out, coupled_accumulator_model, test_accumulator_defs::reset>
>;

template<typename TIME>
using top_model=cadmium::modeling::pdevs::coupled_model<TIME, empty_iports, top_oports, top_submodels, empty_eic, top_eoc, top_ic>;

using messages_logger=cadmium::logger::logger<cadmium::logger::logger_message_routing, cadmium::logger::verbatim_formatter, cadmium::logger::cout_sink_provider>;

//LOG ALL TO COUT
using namespace cadmium::logger;
using info=logger<logger_info, cadmium::logger::formatter<float>, cout_sink_provider>;
using debug=logger<logger_debug, cadmium::logger::formatter<float>, cout_sink_provider>;
using state=logger<logger_state, cadmium::logger::formatter<float>, cout_sink_provider>;
using log_messages=logger<logger_messages, cadmium::logger::formatter<float>, cout_sink_provider>;
using routing=logger<logger_message_routing, cadmium::logger::formatter<float>, cout_sink_provider>;
using global_time=logger<logger_global_time, cadmium::logger::formatter<float>, cout_sink_provider>;
using local_time=logger<logger_local_time, cadmium::logger::formatter<float>, cout_sink_provider>;
using log_all=multilogger<info, debug, state, log_messages, routing, global_time, local_time>;


int main() {
    auto start = hclock::now(); //to measure simulation execution time

    cadmium::engine::runner<float, top_model, log_all> r{0.0};
    r.run_until(100.0);

    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>
            (hclock::now() - start).count();
    cout << "Simulation took:" << elapsed << "sec" << endl;
    return 0;
}

