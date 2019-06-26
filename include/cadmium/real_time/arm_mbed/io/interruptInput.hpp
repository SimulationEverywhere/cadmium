/**
* Ben Earle
* ARSLab - Carleton University
*
* Interrupt Input:
* Model to interface with a interrupt Input pin for Embedded Cadmium.
*/

#ifndef BOOST_SIMULATION_PDEVS_INTERRUPTINPUT_HPP
#define BOOST_SIMULATION_PDEVS_INTERRUPTINPUT_HPP

#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/modeling/dynamic_model.hpp>

#include <limits>
#include <math.h>
#include <assert.h>
#include <memory>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <algorithm>
#include <limits>
#include <random>

#ifdef RT_ARM_MBED
  //This class will interface with a interrupt input pin.
  #include "../mbed.h"
  using namespace cadmium;
  using namespace std;

  //Port definition
  struct interruptInput_defs{
      struct out : public out_port<bool> { };
  };

  template<typename TIME>
  class InterruptInput {
    using defs=interruptInput_defs; // putting definitions in context

    private:
      cadmium::dynamic::modeling::AsyncEventSubject *_sub;

    public:
    //Parameters to be overwriten when instantiating the atomic model
    InterruptIn* intPin;

    // default constructor
    InterruptInput() noexcept {
      MBED_ASSERT(false);
      throw std::logic_error("Input atomic model requires a pin definition");
    }
    InterruptInput(cadmium::dynamic::modeling::AsyncEventSubject* sub, PinName pin) {
      intPin = new InterruptIn(pin);
      intPin->rise(callback(sub, &cadmium::dynamic::modeling::AsyncEventSubject::notify));
      intPin->fall(callback(sub, &cadmium::dynamic::modeling::AsyncEventSubject::notify));
      state.output = intPin->read();
      state.last = state.output;
      state.prop = true;
    }

    // state definition
    struct state_type{
      bool output;
      bool last;
      bool prop;
    };
    state_type state;

    // ports definition
    using input_ports=std::tuple<>;
    using output_ports=std::tuple<typename defs::out>;

    // internal transition
    void internal_transition() {
      state.prop = false;
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        state.prop = true;
        state.last = state.output;
        state.output = (intPin->read() == 1);
      }

    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), std::move(mbs));
    }

    // output function
    typename make_message_bags<output_ports>::type output() const {
      typename make_message_bags<output_ports>::type bags;
      if(state.last != state.output) {
        bool out = state.output;
        get_messages<typename defs::out>(bags).push_back(out);
      }
      return bags;
    }

    // time_advance function
    TIME time_advance() const {
      if(state.prop){
        return TIME("00:00:00:00");
      }

      return std::numeric_limits<TIME>::infinity();
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename InterruptInput<TIME>::state_type& i) {
      os << "Input Pin: " << (i.output ? 1 : 0);
      return os;
    }
  };
#else
  #include <cadmium/io/iestream.hpp>
  using namespace cadmium;
  using namespace std;

  //Port definition
  struct interruptInput_defs{
      struct out : public out_port<bool> { };
  };

  template<typename TIME>
  class InterruptInput : public iestream_input<bool,TIME, interruptInput_defs>{
    public:
      InterruptInput() = default;
      InterruptInput(cadmium::dynamic::modeling::AsyncEventSubject *sub, const char* file_path) : iestream_input<bool,TIME, interruptInput_defs>(file_path) {}
  };
#endif // RT_ARM_MBED

#endif // BOOST_SIMULATION_PDEVS_INTERRUPTINPUT_HPP
