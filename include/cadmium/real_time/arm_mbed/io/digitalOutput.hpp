/**
* Ben Earle
* ARSLab - Carleton University
*
* Digital Output:
* Model to interface with a digital output pin for Embedded Cadmium.
*/


#ifndef BOOST_SIMULATION_PDEVS_DIGITALOUTPUT_HPP
#define BOOST_SIMULATION_PDEVS_DIGITALOUTPUT_HPP

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
  struct digitalOutput_defs{
    struct in : public in_port<bool> {};
  };

  template<typename TIME>
  class DigitalOutput {
  using defs=digitalOutput_defs; // putting definitions in context
  public:
    //Parameters to be overwriten when instantiating the atomic model
    DigitalOut* digiPin;
    
    // default c onstructor
    DigitalOutput() noexcept{
      MBED_ASSERT(false);
      throw std::logic_error("Output atomic model requires a pin definition");
    }

    DigitalOutput(PinName pin) {
      state.output = false;
      digiPin = new DigitalOut(pin);
    }

    // state definition
    struct state_type{
      bool output;
    }; 
    state_type state;
    
    // ports definition
    using input_ports=std::tuple<typename defs::in>;
    using output_ports=std::tuple<>;

    // internal transition
    void internal_transition() {}

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) { 
      for(const auto &x : get_messages<typename defs::in>(mbs)){
        state.output = x;
      }
      digiPin->write(state.output ? 1 : 0);
    }

    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), std::move(mbs));
    }

    // output function
    typename make_message_bags<output_ports>::type output() const {
      typename make_message_bags<output_ports>::type bags;
      return bags;
    }

    // time_advance function
    TIME time_advance() const {     
      return std::numeric_limits<TIME>::infinity();
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename DigitalOutput<TIME>::state_type& i) {
      os << "Pin Out: " << (i.output ? 1 : 0); 
      return os;
    }
  };     
#else
  #include <cadmium/io/oestream.hpp>
  using namespace cadmium;
  using namespace std;

  //Port definition
  struct digitalOutput_defs{
      struct in : public in_port<bool> {};
  };

  template<typename TIME>
  class DigitalOutput : public oestream_output<bool,TIME, digitalOutput_defs>{
    public:
      DigitalOutput() = default;
      DigitalOutput(const char* file_path) : oestream_output<bool,TIME, digitalOutput_defs>(file_path) {}
  };

#endif //RT_ARM_MBED

#endif // BOOST_SIMULATION_PDEVS_DIGITALOUTPUT_HPP