// Filter.hpp
//
// Top-hat (box) filter used to compare filtered DNS results with LES.
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#pragma once

#include <vector>

namespace burgers {

// Returns the box-filtered field: each point is the average of the
// (width + 1) neighbouring points centred on it, e.g. for width = 4:
//     u_bar[i] = (u[i-2] + u[i-1] + u[i] + u[i+1] + u[i+2]) / 5
// Indices outside the domain wrap around (periodic boundaries).
std::vector<double> box_filter(const std::vector<double>& u, int width);

} // namespace burgers
