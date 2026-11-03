// BurgersSolver.cpp
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#include "burgers/BurgersSolver.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

namespace burgers {

BurgersSolver::BurgersSolver(Grid grid, double nu, std::unique_ptr<SGSModel> model)
    : grid_(std::move(grid)),
      nu_(nu),
      model_(std::move(model)),
      nu_t_(grid_.size(), 0.0),
      dudt_(grid_.size(), 0.0)
{
    if (!(nu_ > 0.0)) throw std::invalid_argument("BurgersSolver: nu must be > 0");
    if (!model_)      throw std::invalid_argument("BurgersSolver: model must not be null");
    model_->eddy_viscosity(grid_, grid_.u(), nu_t_);
}

void BurgersSolver::rhs(const std::vector<double>& u, const std::vector<double>& nu_t,
                        std::vector<double>& dudt) const
{
    const std::size_t n = u.size();
    dudt[0] = 0.0;
    dudt[n - 1] = 0.0;
    for (std::size_t i = 1; i + 1 < n; ++i) {
        dudt[i] = -u[i] * grid_.ddx(u, i) + (nu_ + nu_t[i]) * grid_.d2dx2(u, i);
    }
}

double BurgersSolver::stable_dt() const
{
    const double dx = grid_.dx();
    const std::vector<double>& u = grid_.u();

    // One pass for the extremes; the limits below then need no per-point
    // divisions. Using min(nu_eff) with max(u^2) makes the advection-diffusion
    // limit slightly conservative when nu_t varies (LES); for DNS it is exact.
    double nu_eff_max = nu_;
    double nu_eff_min = std::numeric_limits<double>::infinity();
    double u2_max = 0.0;
    for (std::size_t i = 0; i < u.size(); ++i) {
        const double nu_eff = nu_ + nu_t_[i];
        nu_eff_max = std::max(nu_eff_max, nu_eff);
        nu_eff_min = std::min(nu_eff_min, nu_eff);
        u2_max = std::max(u2_max, u[i] * u[i]);
    }

    double dt = 0.5 * dx * dx / nu_eff_max;                    // diffusion
    if (u2_max > 0.0) {
        dt = std::min(dt, dx / std::sqrt(u2_max));             // convective CFL <= 1
        dt = std::min(dt, 2.0 * nu_eff_min / u2_max);          // advection-diffusion (FTCS)
    }
    return dt;
}

void BurgersSolver::step(double dt)
{
    if (!(dt > 0.0)) throw std::invalid_argument("BurgersSolver::step: dt must be > 0");

    std::vector<double>& u = grid_.u();
    rhs(u, nu_t_, dudt_);

    bool finite = true;
    for (std::size_t i = 1; i + 1 < u.size(); ++i) {
        u[i] += dt * dudt_[i];
        finite = finite && std::isfinite(u[i]);
    }

    time_ += dt;
    ++steps_;

    if (!finite) {
        throw std::runtime_error("Solution diverged (non-finite values) at step "
                                 + std::to_string(steps_) + ", t = " + std::to_string(time_));
    }
    model_->eddy_viscosity(grid_, u, nu_t_);   // ready for the next step
}

double BurgersSolver::kinetic_energy() const
{
    const std::vector<double>& u = grid_.u();
    const std::size_t n = u.size();
    double sum = 0.5 * (u[0] * u[0] + u[n - 1] * u[n - 1]);
    for (std::size_t i = 1; i + 1 < n; ++i) sum += u[i] * u[i];
    return 0.5 * sum * grid_.dx();
}

double BurgersSolver::max_abs_u() const
{
    double m = 0.0;
    for (double v : grid_.u()) m = std::max(m, std::abs(v));
    return m;
}

double step_toward(double dt_stable, double remaining)
{
    if (remaining <= dt_stable * (1.0 + 1e-9)) return remaining;
    if (remaining < 2.0 * dt_stable)           return 0.5 * remaining;
    return dt_stable;
}

} // namespace burgers
