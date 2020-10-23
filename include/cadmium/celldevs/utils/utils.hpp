/**
 * Copyright (c) 2020, Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
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

#ifndef CADMIUM_CELLDEVS_UTILS_HPP
#define CADMIUM_CELLDEVS_UTILS_HPP

#include <vector>
#include <iostream>
#include <functional>
#include <boost/functional/hash.hpp>

/**
 * Auxiliary function for printing vectors.
 * @tparam X vector content type.
 * @param os output stream.
 * @param v vector to be printed.
 * @return output stream containing the printed values of the vector.
 */
template <typename X>
std::ostream &operator << (std::ostream &os, std::vector<X> const &v) {
    os << "(";
    std::string separator;
    for (auto x : v) {
        os << separator << x;
        separator = ",";
    }
    os << ")";
    return os;
}

/// Hash function for enabling sequences to be keys of unordered maps
template <typename SEQ>
[[maybe_unused]] std::size_t hash_value(SEQ const &seq) {
    return boost::hash_range(seq.begin(), seq.end());
}

/// Specialization of std::hash function for vectors
template<typename X>
struct [[maybe_unused]] std::hash<std::vector<X>> : boost::hash<std::vector<X>> {};

#endif //CADMIUM_CELLDEVS_UTILS_HPP
