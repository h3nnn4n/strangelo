# Strangelo - Computational Art Explorer

[![clang-format](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/clang-format-check.yml/badge.svg?branch=main)](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/clang-format-check.yml)
[![clang-tidy](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/clang-tidy.yml/badge.svg)](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/clang-tidy.yml)
[![cppcheck](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/cppcheck.yml/badge.svg)](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/cppcheck.yml)
[![cpplint](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/cpplint.yml/badge.svg)](https://github.com/h3nnn4n/raytracer-adventures/actions/workflows/cpplint.yml)

A graphical explorer for strange attractors and other visual computational art. This project started as a raytracer but has evolved to include other visual systems such as Clifford attractors.

## Features

- Clifford strange attractor visualization
- Realtime rendering with OpenGL
- Interactive GUI for parameter adjustments
- "High-performance" rendering (Kinda)

## Building and Running

### Prerequisites

- C/C++ compiler with C99 and C++11 support (GCC or Clang recommended)
- OpenGL 4.6+
- The following dependencies (included as submodules):
  - GLFW
  - GLAD
  - stb
  - PCG random
  - Dear ImGui
  - cglm
  - glm

### Setup

1. Clone the repository with submodules:
   ```bash
   git clone https://github.com/h3nnn4n/strangelo.git
   cd strangelo
   git submodule update --init --recursive
   ```

2. Prepare the dependencies:
   ```bash
   # Build GLFW
   cd deps/glfw
   mkdir -p build && cd build
   cmake .. -DBUILD_SHARED_LIBS=ON
   make
   cd ../..

   # Build cJSON
   cd deps/cJSON
   mkdir -p build && cd build
   cmake ..
   make
   cd ../../..
   ```

3. Build the project:
   ```bash
   make
   ```

4. Run the application:
   ```bash
   make run
   ```

## Clifford Attractors

This project now features Clifford strange attractors, which are visualized using the iterative function:

```
x_{n+1} = sin(a*y_n) + c*cos(a*x_n)
y_{n+1} = sin(b*x_n) + d*cos(b*y_n)
```

Where a, b, c, and d are parameters that can be adjusted to create different patterns. The implementation includes:

- Real-time iteration of the attractor equations
- Density-based rendering with customizable parameters
- Automatic detection of chaotic (interesting) parameter values
- Ability to reset and randomize parameters

### User Interface

The GUI provides real-time control over:

- Clifford attractor parameters (a, b, c, d)
- Randomization of parameters with automatic detection of chaotic patterns
- Gamma adjustment for visualization

## Screenshots

*Screenshots would be displayed here*

## Future Plans

- Add more strange attractors (Lorenz, Henon, etc.)
- Implement colorization options and color mapping
- Add export to image functionality for saving interesting patterns
- Create parameter presets for notable attractors
- Add real-time parameter animation
- Implement 3D versions of attractors

## License

All code outside of the `deps` folder is under the [MIT](LICENSE). Dependencies in the `deps` directory maintain their original licenses.

Some portions of the code were adapted from existing sources with proper attribution provided where applicable.
