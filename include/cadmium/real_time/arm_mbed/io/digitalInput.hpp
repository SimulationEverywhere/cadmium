/**
* Ben Earle
* ARSLab - Carleton University
*
* Digital Input:
* Model to interface with a digital Input pin for Embedded Cadmium.
*/

#ifndef BOOST_SIMULATION_PDEVS_DIGITALINPUT_HPP
#define BOOST_SIMULATION_PDEVS_DIGITALINPUT_HPP

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
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
  //This class will interface with a digital input pin.
  #include "../mbed.h"

  using namespace cadmium;
  using namespace std;

  //Port definition
  struct digitalInput_defs{
      struct out : public out_port<bool> { };
  };

  template<typename TIME>
  class DigitalInput {
    using defs=digitalInput_defs; // putting definitions in context

    public:
    //Parameters to be overwriten when instantiating the atomic model
    DigitalIn* digiPin;
    TIME   pollingRate;
    // default constructor
    DigitalInput() noexcept {
      MBED_ASSERT(false);
      throw std::logic_error("Input atomic model requires a pin definition");
    }
    DigitalInput(PinName pin) {
      new (this) DigitalInput(pin, TIME("00:00:00:100"));
    }
    DigitalInput(PinName pin, TIME rate) {
      pollingRate = rate;
      digiPin = new DigitalIn(pin);
      state.output = digiPin->read();
      state.last = state.output;
    }

    // state definition
    struct state_type{
      bool output;
      bool last;
    }; 
    state_type state;

    // ports definition
    using input_ports=std::tuple<>;
    using output_ports=std::tuple<typename defs::out>;

    // internal transition
    void internal_transition() {
      state.last = state.output;
      state.output = digiPin->read() == 1;
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) { 
      MBED_ASSERT(false);
      throw std::logic_error("External transition called in a model with no input ports");
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
        return pollingRate;
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename DigitalInput<TIME>::state_type& i) {
      os << "Input Pin: " << (i.output ? 1 : 0); 
      return os;
    }
  };     
#else
  #include <cadmium/io/iestream.hpp>
  using namespace cadmium;
  using namespace std;

  //Port definition
  struct digitalInput_defs{
      struct out : public out_port<bool> { };
  };

  template<typename TIME>
  class DigitalInput : public iestream_input<bool,TIME, digitalInput_defs>{
    public:
      DigitalInput() = default;
      DigitalInput(const char* file_path) : iestream_input<bool,TIME, digitalInput_defs>(file_path) {}
      DigitalInput(const char* file_path, TIME t) : iestream_input<bool,TIME, digitalInput_defs>(file_path) {}
  };
#endif // RT_ARM_MBED

#endif // BOOST_SIMULATION_PDEVS_DIGITALINPUT_HPP