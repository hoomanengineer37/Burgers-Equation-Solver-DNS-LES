// Grid.cpp
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#include "burgers/Grid.hpp"

#include <stdexcept>

namespace burgers {

Grid::Grid(std::size_t n_points, double x_min, double x_max)
    : x_min_(x_min),
      dx_(0.0),
      u_(n_points, 0.0)
{
    if (n_points < 3) {
        throw std::invalid_argument("Grid: at least 3 points are required");
    }
    if (!(x_max > x_min)) {
        throw std::invalid_argument("Grid: x_max must be greater than x_min");
    }
    dx_ = (x_max - x_min) / static_cast<double>(n_points - 1);
}

} // namespace burgers
