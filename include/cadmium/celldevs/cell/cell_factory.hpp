//
// Created by Román Cárdenas Rodríguez on 17/06/2020.
//

#ifndef CADMIUM_CELL_FACTORY_HPP
#define CADMIUM_CELL_FACTORY_HPP

#include <string>
#include <exception>
#include <cadmium/celldevs/cell/cell.hpp>

namespace cadmium::celldevs {
    template<typename T, typename C, typename S, typename V=int, typename C_HASH=std::hash<C>>
    class cell_factory {
    public:
        template<typename... Args>
        cell <T, C, S, V, C_HASH> *create_cell(std::string const &cell_type, Args &&... args) {};
    };
}

#endif //CADMIUM_CELL_FACTORY_HPP
