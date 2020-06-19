//
// Created by Román Cárdenas Rodríguez on 18/06/2020.
//

#ifndef CADMIUM_COUTRY_CELL_FACTORY_HPP
#define CADMIUM_COUTRY_CELL_FACTORY_HPP

#include <cadmium/celldevs/cell/cell_factory.hpp>
#include "country_cell.hpp"

template <typename T>
class country_cell_factory: public cell_factory<T, std::string, int, int> {
public:
    template<typename... Args>
    cell<T, std::string, int, int>* create_cell(std::string const &cell_type, Args&&... args) {
        if (cell_type == "country")
            return new country_cell<T>(std::forward<Args>(args)...);
        else throw std::bad_typeid();
    }
};

#endif //CADMIUM_COUTRY_CELL_FACTORY_HPP
