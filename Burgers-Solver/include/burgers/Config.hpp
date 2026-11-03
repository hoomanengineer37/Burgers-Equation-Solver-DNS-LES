// Config.hpp
//
// All run settings in one place. Settings come from defaults, then an
// optional config file (--config), then command-line flags, in that order;
// each layer overrides the previous one.
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>

namespace burgers {

enum class Model { DNS, LES };

const char* model_name(Model m);

struct SimulationConfig {
    double      nu           = 0.001;      // kinematic viscosity (Re = 1/nu)
    double      t_final      = 1.0;        // final (dimensionless) time
    std::size_t n_points     = 1601;       // grid points (ignored if input_file is set)
    Model       model        = Model::DNS;
    double      Cs           = 0.3;        // Smagorinsky constant (LES only)
    int         filter_width = 4;          // box-filter width in cells (DNS filtering)
    std::string input_file;                // empty: u(x,0) = sin(pi x)
    int         n_outputs    = 30;         // number of snapshots, evenly spaced in time
    std::string output_dir   = "results";
    int         series_every = 10;         // write time_series.csv every N steps
    int         n_progress   = 10;         // progress lines printed over the run
    bool        quiet        = false;      // print only errors and the final summary

    // Throws std::invalid_argument describing the first invalid setting.
    void validate() const;

    // Writes the settings in config-file format, so a run can be repeated
    // exactly with --config.
    void write(std::ostream& os) const;
};

enum class ParseResult { Run, HelpShown };

// Fills `config` from argv. Returns HelpShown if --help was given.
// Throws std::invalid_argument on unknown options or bad values.
ParseResult parse_command_line(int argc, char** argv, SimulationConfig& config);

// Reads key = value lines from a file into `config` ('#' starts a comment).
void read_config_file(const std::string& path, SimulationConfig& config);

void print_usage(std::ostream& os, const char* program_name);

} // namespace burgers
