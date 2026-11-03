// Grid.hpp
//
// Uniform 1D grid holding the solution u(x) and second-order central
// finite-difference operators.
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#pragma once

#include <cstddef>
#include <vector>

namespace burgers {

class Grid {
public:
    // Uniform grid of n_points on [x_min, x_max], with u initialized to zero.
    Grid(std::size_t n_points, double x_min, double x_max);

    std::size_t size() const { return u_.size(); }
    double dx() const { return dx_; }
    double x(std::size_t i) const { return x_min_ + static_cast<double>(i) * dx_; }

    std::vector<double>&       u()       { return u_; }
    const std::vector<double>& u() const { return u_; }

    // Central first derivative df/dx at interior point i, O(dx^2).
    double ddx(const std::vector<double>& f, std::size_t i) const
    {
        return (f[i + 1] - f[i - 1]) / (2.0 * dx_);
    }

    // Central second derivative d2f/dx2 at interior point i, O(dx^2).
    double d2dx2(const std::vector<double>& f, std::size_t i) const
    {
        return (f[i + 1] - 2.0 * f[i] + f[i - 1]) / (dx_ * dx_);
    }

private:
    double x_min_;
    double dx_;
    std::vector<double> u_;
};

} // namespace burgers
