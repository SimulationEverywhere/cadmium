/**
* Ben Earle
* ARSLab - Carleton University
*
* Majority Vote:
* Model will take digital inputs and output the majority value, for Embedded Cadmium.
*/


#ifndef AVERAGEINPUT_HPP
#define AVERAGEINPUT_HPP

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

using namespace cadmium;
using namespace std;
#define MAX_NUMBER_OF_IN_PORTS 10
//Port definition
struct averageInput_defs{
  struct in1 : public in_port<float>{};
  struct in2 : public in_port<float>{};
  struct in3 : public in_port<float>{};
  struct in4 : public in_port<float>{};
  struct in5 : public in_port<float>{};
  struct in6 : public in_port<float>{};
  struct in7 : public in_port<float>{};
  struct in8 : public in_port<float>{};
  struct in9 : public in_port<float>{};
  struct in10: public in_port<float>{};
  struct out : public out_port<float> {};
};

template<typename TIME>
class AverageInput {
  using defs=averageInput_defs; // putting definitions in context
  public:
    //Parameters to be overwriten when instantiating the atomic model
  
    // default constructor
    AverageInput() noexcept{
        cout << "Must specify the number of in ports.";
        assert(false);
    }

    AverageInput(int inCount) noexcept{
      assert(inCount <= MAX_NUMBER_OF_IN_PORTS);
      for(int i = 0; i < MAX_NUMBER_OF_IN_PORTS; i++){
        state.in[i] = false;
      }
      state.inCount = inCount;
      state.output = false;
      state.last = false;
      state.prop = true;
    }

    // state definition
    struct state_type{
      float in [MAX_NUMBER_OF_IN_PORTS];
      int inCount;
      float output;
      float prop;
      float last;
    };
    state_type state;
    
    // ports definition
    using input_ports=std::tuple< typename defs::in1, typename defs::in2, typename defs::in3, typename defs::in4, typename defs::in5,
                                  typename defs::in6, typename defs::in7, typename defs::in8, typename defs::in9, typename defs::in10>;
    using output_ports=std::tuple<typename defs::out>;

    // internal transition
    void internal_transition() {
      state.prop = false;      
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) { 
      // Get inputs
      for(const auto &x : get_messages<typename defs::in1>(mbs))  state.in[0] = x;
      for(const auto &x : get_messages<typename defs::in2>(mbs))  state.in[1] = x;
      for(const auto &x : get_messages<typename defs::in3>(mbs))  state.in[2] = x;
      for(const auto &x : get_messages<typename defs::in4>(mbs))  state.in[3] = x;
      for(const auto &x : get_messages<typename defs::in5>(mbs))  state.in[4] = x;
      for(const auto &x : get_messages<typename defs::in6>(mbs))  state.in[5] = x;
      for(const auto &x : get_messages<typename defs::in7>(mbs))  state.in[6] = x;
      for(const auto &x : get_messages<typename defs::in8>(mbs))  state.in[7] = x;
      for(const auto &x : get_messages<typename defs::in9>(mbs))  state.in[8] = x;
      for(const auto &x : get_messages<typename defs::in10>(mbs)) state.in[9] = x;

      // Count true inputs
      float sum = 0;
      for(int i = 0; i < state.inCount; i++)
        sum += state.in[i];

      state.output = sum / state.inCount;
      //If the output changed then propogate outputs
      if(state.last != state.output) state.prop = true;
      state.last = state.output;
    }

    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), std::move(mbs));
    }

    // output function
    typename make_message_bags<output_ports>::type output() const {
      typename make_message_bags<output_ports>::type bags;
      float out = state.output;
      get_messages<typename defs::out>(bags).push_back(out);
      return bags;
    }

    // time_advance function
    TIME time_advance() const {     
      if(state.prop) 
        return TIME("00:00:00");
      return std::numeric_limits<TIME>::infinity();
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename AverageInput<TIME>::state_type& i) {
      os << "Average Input Out: " << (i.output ? 1 : 0); 
      return os;
    }

};

#endif // AVERAGEINPUT_HPP