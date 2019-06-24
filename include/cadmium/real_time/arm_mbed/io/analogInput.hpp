/**
* Ben Earle
* ARSLab - Carleton University
*
* Analog Input:
* Model to interface with a analog Input pin for Embedded Cadmium.
*/
#ifndef BOOST_SIMULATION_PDEVS_ANALOGINPUT_HPP
#define BOOST_SIMULATION_PDEVS_ANALOGINPUT_HPP

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
  #include "../mbed.h"

  using namespace cadmium;
  using namespace std;

  //Port definition
  struct analogInput_defs{
    struct out : public out_port<float> { };
  };

  template<typename TIME>
  class AnalogInput {
  using defs=analogInput_defs; // putting definitions in context
  public:
    //Parameters to be overwriten when instantiating the atomic model
    AnalogIn* analogPin;
    TIME pollingRate;
    // default c onstructor
    AnalogInput() noexcept{ 
      MBED_ASSERT(false);
      //throw std::logic_error("Input atomic model requires a pin definition");
    }    
    AnalogInput(PinName pin) noexcept{
      new (this) AnalogInput(pin, TIME("00:00:00:100"));
    }
    AnalogInput(PinName pin, TIME rate) noexcept{
      pollingRate = rate;
      analogPin = new AnalogIn(pin);
      state.output = analogPin->read();
      state.last = state.output;
    }
    
    // state definition
    struct state_type{
      float output;
      float last;
    }; 
    state_type state;
    // ports definition

    using input_ports=std::tuple<>;
    using output_ports=std::tuple<typename defs::out>;

    // internal transition
    void internal_transition() {
      state.last = state.output;
      state.output = analogPin->read();
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
      MBED_ASSERT(false); 
      //throw std::logic_error("External transition called in a model with no input ports");
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
        float out = state.output;
        get_messages<typename defs::out>(bags).push_back(out);
      }
      return bags;
    }

    // time_advance function
    TIME time_advance() const {     
      return pollingRate;
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename AnalogInput<TIME>::state_type& i) {
      os << "Analog Input Pin: " << i.output; 
      return os;
    }
  };     
#else
  #include <cadmium/io/iestream.hpp>
  using namespace cadmium;
  using namespace std;

  //Port definition
  struct analogInput_defs{
    struct out : public out_port<float> {};
  };

  template<typename TIME>
  class AnalogInput : public iestream_input<float,TIME, analogInput_defs>{
    public:
      AnalogInput() = default;
      AnalogInput(const char* file_path) : iestream_input<float,TIME, analogInput_defs>(file_path) {}
      AnalogInput(const char* file_path, TIME t) : iestream_input<float,TIME, analogInput_defs>(file_path) {}
  }; 
#endif // RT_ARM_MBED
#endif // BOOST_SIMULATION_PDEVS_ANALOGINPUT_HPP