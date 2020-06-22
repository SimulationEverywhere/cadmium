//
// Created by Román Cárdenas Rodríguez on 22/06/2020.
//

#ifndef CADMIUM_CELLDEVS_COUNTRY_COUPLED_HPP
#define CADMIUM_CELLDEVS_COUNTRY_COUPLED_HPP

#include "cells/country_cell.hpp"
#include "cells/small_country_cell.hpp"
#include <cadmium/celldevs/coupled/cells_coupled.hpp>

using namespace cadmium::celldevs;


template <typename T>
class country_coupled : public cells_coupled<T, std::string, int, int> {
public:

    explicit country_coupled(std::string const &id) : cells_coupled<T, std::string, int, int>(id){}

    template <typename X>
    using cell_unordered = std::unordered_map<std::string, X>;

    void add_cell_json(std::string const &cell_type, std::string const &cell_id,
                       cell_unordered<int> const &neighborhood, int initial_state, std::string const &delay_id,
                       nlohmann::json const &config) override {
        if (cell_type == "country") {
            auto conf = config.get<typename country_cell<T>::config_type>();
            this->template add_cell<country_cell>(cell_id, neighborhood, initial_state, delay_id, conf);
        } else if (cell_type == "small_country") {
            auto conf = config.get<typename small_country_cell<T>::config_type>();
            this->template add_cell<small_country_cell>(cell_id, neighborhood, initial_state, delay_id, conf);
        } else throw std::bad_typeid();
    }
};

#endif //CADMIUM_COUNTRY_COUPLED_HPP
