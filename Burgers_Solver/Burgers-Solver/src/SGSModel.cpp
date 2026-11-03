// SGSModel.cpp
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#include "burgers/SGSModel.hpp"

#include "burgers/Config.hpp"
#include "burgers/Grid.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace burgers {

void NoModel::eddy_viscosity(const Grid& /*grid*/, const std::vector<double>& /*u*/,
                             std::vector<double>& nu_t) const
{
    std::fill(nu_t.begin(), nu_t.end(), 0.0);
}

Smagorinsky::Smagorinsky(double Cs, double Delta)
    : Cs_(Cs), Delta_(Delta)
{
    if (Cs < 0.0)      throw std::invalid_argument("Smagorinsky: Cs must be >= 0");
    if (!(Delta > 0.0)) throw std::invalid_argument("Smagorinsky: Delta must be > 0");
}

void Smagorinsky::eddy_viscosity(const Grid& grid, const std::vector<double>& u,
                                 std::vector<double>& nu_t) const
{
    const double coeff = 2.0 * Cs_ * (Delta_ * Delta_);
    const std::size_t n = u.size();

    nu_t[0] = 0.0;
    nu_t[n - 1] = 0.0;
    for (std::size_t i = 1; i + 1 < n; ++i) {
        nu_t[i] = coeff * std::abs(grid.ddx(u, i));
    }
}

std::unique_ptr<SGSModel> make_sgs_model(const SimulationConfig& config, const Grid& grid)
{
    switch (config.model) {
    case Model::DNS: return std::make_unique<NoModel>();
    case Model::LES: return std::make_unique<Smagorinsky>(config.Cs, grid.dx());
    }
    throw std::logic_error("make_sgs_model: unhandled model");
}

} // namespace burgers
