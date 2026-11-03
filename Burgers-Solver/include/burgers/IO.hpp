// IO.hpp
//
// Input (initial condition) and output (CSV snapshots, time series, progress
// and run summary).
//
// Output folder contents:
//   run_config.cfg    exact settings of the run (re-run with --config)
//   u_0000.csv ...    snapshots at evenly spaced times; columns
//                       DNS: x,u,u_filtered     LES: x,u,nu_t
//   time_series.csv   step,t,dt,E,max_abs_u,max_nu_t
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#pragma once

#include "burgers/Config.hpp"
#include "burgers/Grid.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace burgers {

class BurgersSolver;

// Reads u values from the lines after the marker "Variable u" (see
// Examples/sinpi.txt). Throws std::runtime_error on a missing file, missing
// marker, or unparsable value.
std::vector<double> read_initial_profile(const std::string& path);

// Grid on [0, 1] with u(x,0) = sin(pi x), or the profile in
// config.input_file (whose length then sets the number of points).
Grid make_initial_grid(const SimulationConfig& config);

class OutputWriter {
public:
    // Creates the output folder, saves run_config.cfg and writes the initial
    // snapshot and time-series row.
    OutputWriter(const SimulationConfig& config, const BurgersSolver& solver);

    // Time of the next snapshot; the time loop lands exactly on it.
    double next_snapshot_time() const;

    // True once the final snapshot (at t_final) has been written.
    bool done() const;

    // Call after every step: writes time-series rows, progress lines and any
    // snapshot that is due.
    void after_step(const BurgersSolver& solver, double dt);

    // Prints the run summary (steps, wall time, energy decay, output folder).
    void finish(const BurgersSolver& solver) const;

private:
    void write_snapshot(const BurgersSolver& solver);
    void write_series_row(const BurgersSolver& solver, double dt);
    void print_progress(const BurgersSolver& solver, double dt) const;

    SimulationConfig config_;
    std::filesystem::path dir_;
    std::ofstream series_;
    int  snapshot_index_ = 0;     // index of the next snapshot to write
    int  progress_index_ = 1;     // index of the next progress line
    long last_series_step_ = -1;
    double energy0_ = 0.0;
    double time_tolerance_;
    std::chrono::steady_clock::time_point start_;
};

} // namespace burgers
