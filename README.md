## Turbulence modelling

This C++ numerical model was developed to show the performance of Direct Numerical Simulation (DNS) and Large Eddy Simulation (LES) for one-dimensional flows. It solves the viscous Burgers' equation, a standard 1D model problem for turbulence, using second-order finite differences and adaptive time stepping. The solver runs either as a fully resolved DNS or as an LES with a Smagorinsky subgrid-scale model, and varying the diffusion coefficient covers both laminar and turbulent-like regimes. This work is part of my broader research on the numerical study of turbulent flows.

## Code structure

The solver is written in modern C++17 and organised as a reusable library plus a small command-line application. It uses object-oriented design with runtime polymorphism (an abstract SGS-model interface and a factory), RAII and smart pointers (`std::unique_ptr`) for memory safety, `std::vector` instead of raw arrays, `enum class`, `const`-correctness, exception-based error handling, and standard-library features such as `std::filesystem`, `std::chrono` and `std::to_chars`. It builds with CMake and compiles cleanly under strict warnings (`-Wall -Wextra -Wpedantic -Wconversion`) with GCC and Clang. It has also been run under AddressSanitizer and UndefinedBehaviorSanitizer.

```
include/burgers/   public headers        src/   implementations
apps/main.cpp      command-line driver   cases/ example run configurations
```

- **`Grid`**: uniform 1D grid holding the solution, with second-order finite-difference operators.
- **`BurgersSolver`**: the physics (right-hand side), stability-limited time step, time integration, and diagnostics (kinetic energy, max |u|, divergence detection).
- **`SGSModel`**: abstract subgrid-scale model interface, implemented by `NoModel` (DNS) and `Smagorinsky` (LES). New models can be added without changing the solver.
- **`Filter`**: box filter used to produce the filtered DNS for comparison with LES.
- **`Config`**: run settings read from a config file and command-line flags, with validation. Each run's settings are saved for reproducibility.
- **`IO`**: initial conditions and output (CSV snapshots, energy time series, progress log and run summary).
- **`main.cpp`**: reads the settings, builds the grid, model and solver, and advances the solution to each output time.

## Results

Graph A compares DNS (circles) and LES (squares) velocity profiles at t = 0, 0.3, 0.5 and 1.0 on the same 1601-point grid. The two solutions agree closely, with a relative difference below 0.15%. The only deviation is in a few grid cells next to the wall at x = 1, where the Smagorinsky model adds a small eddy viscosity (at most 10% of the molecular viscosity) and slightly smooths the steep front. This validates that the LES implementation matches DNS at high grid resolution.
<img width="1078" height="545" alt="image" src="https://github.com/user-attachments/assets/81f907ce-d614-4857-8d99-9e2d02a5c378" />

Graph B zooms in on the front at the wall (x ≥ 0.985) at t = 0.5, when it is steepest, and compares DNS, LES and the filtered DNS (a box filter of width 4Δx). Here the velocity drops from about 1.0 to zero over roughly 0.004, which is only about six grid cells. LES follows DNS closely and is only slightly smoother: its steepest gradient is about 5% lower (−470 vs −496), and the largest difference, 0.023, occurs two cells from the wall. The filtered DNS is smoother still, so LES lies between the resolved and filtered solutions. This shows that the Smagorinsky model adds dissipation only where it is needed, in the steepest part of the front, while preserving the shape and position of the boundary layer.

<img width="956" height="545" alt="image" src="https://github.com/user-attachments/assets/2ee9d585-8ee0-4406-a664-cd5265413883" />
