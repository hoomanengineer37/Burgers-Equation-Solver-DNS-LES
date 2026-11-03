// BurgersSolver.hpp
//
// Solves the 1D viscous Burgers equation with an optional SGS model,
//
//     du/dt = -u du/dx + (nu + nu_t) d2u/dx2,
//
// by the method of lines: rhs() is the spatial operator (second-order central
// differences) and step() is the time integrator (forward Euler). The two are
// kept separate so the integrator can be upgraded without touching the
// physics. Boundary values u(0) and u(1) are held fixed (Dirichlet).
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#pragma once

#include "burgers/Grid.hpp"
#include "burgers/SGSModel.hpp"

#include <cstddef>
#include <memory>
#include <vector>

namespace burgers {

class BurgersSolver {
public:
    BurgersSolver(Grid grid, double nu, std::unique_ptr<SGSModel> model);

    // Spatial operator: fills dudt (interior points) for velocity u and eddy
    // viscosity nu_t. Boundary entries are zero, so boundary values stay fixed.
    void rhs(const std::vector<double>& u, const std::vector<double>& nu_t,
             std::vector<double>& dudt) const;

    // Largest stable forward-Euler step for the current solution: the minimum
    // of the diffusive limit dx^2 / (2 nu_eff), the convective CFL limit
    // dx / |u|, and the advection-diffusion limit 2 nu_eff / u^2.
    double stable_dt() const;

    // Advances the solution by dt. Throws std::runtime_error if the solution
    // stops being finite (the run has diverged).
    void step(double dt);

    const Grid& grid() const { return grid_; }
    const std::vector<double>& u() const { return grid_.u(); }
    const std::vector<double>& eddy_viscosity() const { return nu_t_; }
    const SGSModel& model() const { return *model_; }
    double nu() const { return nu_; }
    double time() const { return time_; }
    long   step_count() const { return steps_; }

    // Diagnostics
    double kinetic_energy() const;   // E = 1/2 * integral of u^2 dx (trapezoidal rule)
    double max_abs_u() const;

private:
    Grid grid_;
    double nu_;
    std::unique_ptr<SGSModel> model_;
    std::vector<double> nu_t_;       // eddy viscosity of the current solution
    std::vector<double> dudt_;       // work array
    double time_ = 0.0;
    long steps_ = 0;
};

// Time step for reaching a target time `remaining` ahead without overshooting
// it and without leaving a tiny final step: the last step lands exactly on the
// target, and if fewer than two stable steps remain they are split evenly.
double step_toward(double dt_stable, double remaining);

} // namespace burgers
