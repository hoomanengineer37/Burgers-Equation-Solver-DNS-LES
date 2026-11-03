// IO.cpp
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#include "burgers/IO.hpp"

#include "burgers/BurgersSolver.hpp"
#include "burgers/Filter.hpp"
#include "burgers/Format.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace burgers {

namespace {

constexpr double kPi = 3.14159265358979323846;

// Removes trailing whitespace, including the '\r' left by Windows line endings.
std::string rtrim(std::string s)
{
    const auto end = s.find_last_not_of(" \t\r\n");
    s.erase(end == std::string::npos ? 0 : end + 1);
    return s;
}

std::ofstream open_for_writing(const fs::path& path)
{
    std::ofstream file(path);
    if (!file) throw std::runtime_error("Cannot write file: " + path.string());
    return file;
}

} // namespace

// ---------------------------------------------------------------- input

std::vector<double> read_initial_profile(const std::string& path)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open input file: " + path);
    }

    std::vector<double> values;
    bool in_u_block = false;
    std::string line;

    while (std::getline(file, line)) {
        line = rtrim(line);
        if (!in_u_block) {
            in_u_block = (line == "Variable u");
            continue;
        }
        if (line.empty()) continue;
        try {
            values.push_back(std::stod(line));
        } catch (const std::exception&) {
            throw std::runtime_error("Cannot parse value '" + line + "' in " + path);
        }
    }

    if (!in_u_block) {
        throw std::runtime_error("Marker line 'Variable u' not found in " + path);
    }
    return values;
}

Grid make_initial_grid(const SimulationConfig& config)
{
    if (!config.input_file.empty()) {
        const std::vector<double> profile = read_initial_profile(config.input_file);
        Grid grid(profile.size(), 0.0, 1.0);
        grid.u() = profile;
        return grid;
    }

    Grid grid(config.n_points, 0.0, 1.0);
    for (std::size_t i = 0; i < grid.size(); ++i) {
        grid.u()[i] = std::sin(kPi * grid.x(i));
    }
    return grid;
}

// ---------------------------------------------------------------- output

OutputWriter::OutputWriter(const SimulationConfig& config, const BurgersSolver& solver)
    : config_(config),
      dir_(config.output_dir),
      time_tolerance_(1e-12 * config.t_final),
      start_(std::chrono::steady_clock::now())
{
    fs::create_directories(dir_);
    {
        std::ofstream saved = open_for_writing(dir_ / "run_config.cfg");
        config_.write(saved);   // every result folder records its exact inputs
    }

    series_ = open_for_writing(dir_ / "time_series.csv");
    series_ << "step,t,dt,E,max_abs_u,max_nu_t\n";

    energy0_ = solver.kinetic_energy();

    if (!config_.quiet) {
        const Grid& g = solver.grid();
        std::printf("Burgers solver | model: %s | nu = %g | %zu points, dx = %.3e | t_final = %g\n",
                    solver.model().name().c_str(), solver.nu(), g.size(), g.dx(), config_.t_final);
        std::printf("Snapshots at t = 0 and %d evenly spaced times, written to %s\n\n",
                    config_.n_outputs, dir_.string().c_str());
    }

    write_series_row(solver, 0.0);
    write_snapshot(solver);   // u_0000.csv, the initial condition
}

double OutputWriter::next_snapshot_time() const
{
    return config_.t_final * snapshot_index_ / config_.n_outputs;
}

bool OutputWriter::done() const
{
    return snapshot_index_ > config_.n_outputs;
}

void OutputWriter::after_step(const BurgersSolver& solver, double dt)
{
    const double t = solver.time();

    if (solver.step_count() % config_.series_every == 0) {
        write_series_row(solver, dt);
    }

    // Progress lines at evenly spaced times (one line even if a step crosses several).
    if (progress_index_ <= config_.n_progress &&
        t >= config_.t_final * progress_index_ / config_.n_progress - time_tolerance_) {
        if (!config_.quiet) print_progress(solver, dt);
        while (progress_index_ <= config_.n_progress &&
               t >= config_.t_final * progress_index_ / config_.n_progress - time_tolerance_) {
            ++progress_index_;
        }
    }

    if (!done() && t >= next_snapshot_time() - time_tolerance_) {
        write_snapshot(solver);
        if (done() && last_series_step_ != solver.step_count()) {
            write_series_row(solver, dt);   // always record the final state
        }
    }
}

void OutputWriter::write_snapshot(const BurgersSolver& solver)
{
    char name[32];
    std::snprintf(name, sizeof name, "u_%04d.csv", snapshot_index_);
    std::ofstream file = open_for_writing(dir_ / name);

    const Grid& grid = solver.grid();
    const std::vector<double>& u = solver.u();
    const bool dns = (config_.model == Model::DNS);

    file << "# t = " << format_double(solver.time())
         << ", step = " << solver.step_count()
         << ", model = " << model_name(config_.model)
         << ", nu = " << format_double(solver.nu());
    if (dns) {
        file << ", filter_width = " << config_.filter_width << '\n'
             << "x,u,u_filtered\n";
        const std::vector<double> u_bar = box_filter(u, config_.filter_width);
        for (std::size_t i = 0; i < grid.size(); ++i) {
            file << format_double(grid.x(i)) << ',' << format_double(u[i]) << ','
                 << format_double(u_bar[i]) << '\n';
        }
    } else {
        file << ", Cs = " << format_double(config_.Cs) << '\n'
             << "x,u,nu_t\n";
        const std::vector<double>& nu_t = solver.eddy_viscosity();
        for (std::size_t i = 0; i < grid.size(); ++i) {
            file << format_double(grid.x(i)) << ',' << format_double(u[i]) << ','
                 << format_double(nu_t[i]) << '\n';
        }
    }
    ++snapshot_index_;
}

void OutputWriter::write_series_row(const BurgersSolver& solver, double dt)
{
    const std::vector<double>& nu_t = solver.eddy_viscosity();
    const double max_nu_t = nu_t.empty() ? 0.0 : *std::max_element(nu_t.begin(), nu_t.end());
    series_ << solver.step_count() << ',' << format_double(solver.time()) << ','
            << format_double(dt) << ',' << format_double(solver.kinetic_energy()) << ','
            << format_double(solver.max_abs_u()) << ',' << format_double(max_nu_t) << '\n';
    last_series_step_ = solver.step_count();
}

void OutputWriter::print_progress(const BurgersSolver& solver, double dt) const
{
    const double t = solver.time();
    const double dx = solver.grid().dx();
    const double max_u = solver.max_abs_u();
    std::printf("  t = %8.4f (%3.0f%%)  step %8ld  dt = %.3e  CFL = %.3f  E = %.6f  max|u| = %.4f\n",
                t, 100.0 * t / config_.t_final, solver.step_count(), dt, max_u * dt / dx,
                solver.kinetic_energy(), max_u);
    std::fflush(stdout);
}

void OutputWriter::finish(const BurgersSolver& solver) const
{
    const double seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start_).count();
    const double energy = solver.kinetic_energy();

    std::printf("\nDone: %ld steps to t = %.6g in %.3f s (%.3g steps/s)\n",
                solver.step_count(), solver.time(), seconds,
                seconds > 0.0 ? static_cast<double>(solver.step_count()) / seconds : 0.0);
    std::printf("Kinetic energy: E(0) = %.6g, E(end) = %.6g (%.1f%% dissipated)\n",
                energy0_, energy, energy0_ > 0.0 ? 100.0 * (1.0 - energy / energy0_) : 0.0);
    std::printf("Results in %s (%d snapshots, time_series.csv, run_config.cfg)\n",
                dir_.string().c_str(), snapshot_index_);
}

} // namespace burgers
