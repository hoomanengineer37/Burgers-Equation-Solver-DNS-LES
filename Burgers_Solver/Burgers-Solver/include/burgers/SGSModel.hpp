// SGSModel.hpp
//
// Subgrid-scale (SGS) models: each model computes an eddy viscosity nu_t(x)
// from the resolved velocity. The solver only sees this interface, so a new
// model (e.g. WALE or dynamic Smagorinsky) is a new class plus one line in
// make_sgs_model(), with no change to the solver.
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace burgers {

class Grid;
struct SimulationConfig;

class SGSModel {
public:
    virtual ~SGSModel() = default;

    // Fills nu_t (same size as u) with the eddy viscosity for velocity u.
    // Called once per time step, so the virtual call is not in an inner loop.
    virtual void eddy_viscosity(const Grid& grid, const std::vector<double>& u,
                                std::vector<double>& nu_t) const = 0;

    virtual std::string name() const = 0;
};

// DNS: no model, nu_t = 0 everywhere.
class NoModel final : public SGSModel {
public:
    void eddy_viscosity(const Grid& grid, const std::vector<double>& u,
                        std::vector<double>& nu_t) const override;
    std::string name() const override { return "none (DNS)"; }
};

// Smagorinsky-type model in 1D:  nu_t = 2 Cs Delta^2 |du/dx|.
//
// This keeps the coefficient of the original 2017 code (2 Cs Delta^2). The
// textbook form is (Cs Delta)^2 |du/dx|; changing it is a physics decision
// left for the validation step, and now only touches this class.
class Smagorinsky final : public SGSModel {
public:
    Smagorinsky(double Cs, double Delta);

    void eddy_viscosity(const Grid& grid, const std::vector<double>& u,
                        std::vector<double>& nu_t) const override;
    std::string name() const override { return "Smagorinsky"; }

private:
    double Cs_;
    double Delta_;   // filter width
};

// Builds the model selected in the config.
std::unique_ptr<SGSModel> make_sgs_model(const SimulationConfig& config, const Grid& grid);

} // namespace burgers
