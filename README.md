This C++ numerical model was developed to show the performance of Direct Numerical Simulation (DNS) and Large Eddy Simulation (LES) for one-dimensional flows. It solves the viscous Burgers' equation, a standard 1D model problem for turbulence, using second-order finite differences and adaptive time stepping. The solver runs either as a fully resolved DNS or as an LES with a Smagorinsky subgrid-scale model, and varying the diffusion coefficient covers both laminar and turbulent-like regimes. This work is part of my broader research on the numerical study of turbulent flows.

Graph A compares DNS (circles) and LES (squares) velocity profiles at t = 0, 0.3, 0.5 and 1.0 on the same 1601-point grid. The two solutions agree closely, with a relative difference below 0.15%. The only deviation is in a few grid cells next to the wall at x = 1, where the Smagorinsky model adds a small eddy viscosity (at most 10% of the molecular viscosity) and slightly smooths the steep front. This validates that the LES implementation matches DNS at high grid resolution.
<img width="1078" height="545" alt="image" src="https://github.com/user-attachments/assets/81f907ce-d614-4857-8d99-9e2d02a5c378" />



<img width="956" height="545" alt="image" src="https://github.com/user-attachments/assets/2ee9d585-8ee0-4406-a664-cd5265413883" />
