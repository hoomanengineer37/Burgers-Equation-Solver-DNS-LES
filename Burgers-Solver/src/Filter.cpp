// Filter.cpp
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#include "burgers/Filter.hpp"

#include <cstddef>
#include <stdexcept>

namespace burgers {

std::vector<double> box_filter(const std::vector<double>& u, int width)
{
    if (width < 0) {
        throw std::invalid_argument("box_filter: width must be >= 0");
    }
    const std::size_t n = u.size();
    const std::size_t stencil = static_cast<std::size_t>(width) + 1;
    const std::size_t half = static_cast<std::size_t>(width) / 2;
    if (stencil > n) {
        throw std::invalid_argument("box_filter: filter is wider than the grid");
    }

    std::vector<double> u_bar(n, 0.0);

    for (std::size_t i = 0; i < n; ++i) {
        double sum = 0.0;
        for (std::size_t j = 0; j < stencil; ++j) {
            const std::size_t k = (i + n - half + j) % n;   // periodic wrap-around
            sum += u[k];
        }
        u_bar[i] = sum / static_cast<double>(stencil);
    }
    return u_bar;
}

} // namespace burgers
