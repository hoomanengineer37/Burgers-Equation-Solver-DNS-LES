// main.cpp
//
// Command-line driver for the 1D Burgers DNS/LES solver.
// Run `burgers --help` for the available options.
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#include "burgers/BurgersSolver.hpp"
#include "burgers/Config.hpp"
#include "burgers/IO.hpp"
#include "burgers/SGSModel.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv)
{
    using namespace burgers;

    try {
        SimulationConfig config;
        if (parse_command_line(argc, argv, config) == ParseResult::HelpShown) {
            print_usage(std::cout, argv[0]);
            return 0;
        }
        config.validate();

        Grid grid = make_initial_grid(config);
        auto model = make_sgs_model(config, grid);
        BurgersSolver solver(std::move(grid), config.nu, std::move(model));
        OutputWriter output(config, solver);

        // March to each snapshot time in turn, landing exactly on it.
        while (!output.done()) {
            const double remaining = output.next_snapshot_time() - solver.time();
            const double dt = step_toward(solver.stable_dt(), remaining);
            solver.step(dt);
            output.after_step(solver, dt);
        }
        output.finish(solver);

    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << "\n(run with --help for usage)\n";
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
