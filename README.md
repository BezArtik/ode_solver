# numsol

A command-line utility for numerical integration of systems of ordinary
differential equations (ODEs), with a reusable C++20 library core.

`numsol` solves initial value problems of the form

    dy/dt = f(t, y),   y(t0) = y0

where the right-hand side is given symbolically in a TOML configuration
file. Several explicit Runge-Kutta methods are available, and the
solution is written to CSV (file or stdout).

---

## Table of contents

- [Features](#features)
- [Requirements](#requirements)
- [Building](#building)
- [Usage](#usage)
  - [Configuration file](#configuration-file)
  - [Running](#running)
  - [Examples](#examples)
- [Configuration reference](#configuration-reference)
- [Available methods](#available-methods)
- [Library usage](#library-usage)
- [Project structure](#project-structure)
- [Numerical accuracy](#numerical-accuracy)
- [License](#license)

---

## Features

- **Symbolic RHS** - define the right-hand side as expressions in a TOML
  file, no recompilation needed.
- **Explicit Runge-Kutta methods** - Euler, RK2 (Heun), RK3 (Kutta),
  RK4 (classical).
- **Named parameters** - reusable parameters in RHS expressions.
- **CSV output** - solution to a file or to stdout, full `double`
  precision.
- **Reusable library core** - `numsol::core` and `numsol::methods`
  are independent of TOML, expressions, and CSV. Use them directly in
  your own C++ projects.
- **Modern C++20** - concepts, ranges, `std::format`, designated
  initializers.

---

## Requirements

- Compiler with C++23 support:
  - Clang 16+
  - GCC 13+
  - MSVC 19.30+
- CMake 3.24+
- Ninja (recommended) or another CMake generator

Dependencies are fetched automatically via CMake `FetchContent`:

- [toml++](https://github.com/marzer/tomlplusplus) - TOML parsing
- [ExprTk](https://github.com/ArashPartow/exprtk) - expression evaluation

---

## Building

    cmake --preset clang-release
    cmake --build --preset clang-release

The executable is placed in `out/build/clang-release/src/numsol_app`.

For a debug build with sanitizers:

    cmake --preset clang-debug
    cmake --build --preset clang-debug

---

## Usage

### Configuration file

All parameters are provided in a TOML file. Below is a minimal example
that solves the harmonic oscillator

    y1' = y2
    y2' = -omega^2 * y1

with `y1(0) = 1`, `y2(0) = 0`, `omega = 1`, on the interval `[0, 10]`:

    [problem]
    t0    = 0.0
    t_end = 10.0
    y0    = [1.0, 0.0]

    [problem.rhs]
    y1 = "y2"
    y2 = "-omega * omega * y1"

    [problem.params]
    omega = 1.0

    [solver]
    method = "rk4"
    h0     = 0.001

    [output]
    path = "out/oscillator.csv"

### Running

    numsol_app configs/oscillator.toml

If `output.path` is empty or the `[output]` section is omitted, the
solution is written to stdout:

    numsol_app configs/oscillator.toml > solution.csv

### Examples

The `configs/` directory contains ready-to-run examples:

| File | Description |
|------|-------------|
| `oscillator.toml` | Harmonic oscillator, 2 equations |
| `exponential.toml` | `y' = y`, 1 equation, known analytic solution |
| `lorenz.toml` | Lorenz system, 3 equations, chaotic |

Run any of them with:

    numsol_app configs/<name>.toml

---

## Configuration reference

### `[problem]`

| Key | Type | Required | Default | Description |
|-----|------|----------|---------|-------------|
| `t0` | float | no | `0.0` | Initial time |
| `t_end` | float | yes | - | Final time |
| `y0` | array of float | yes | - | Initial state vector |

### `[problem.rhs]`

A table mapping variable names `y1, y2, ..., yN` to expression strings.
The names must form a contiguous sequence starting from `y1`. The number
of expressions must equal `y0.size()`.

Available in expressions:

- `y1, y2, ..., yN` - state components
- `t` - current time
- any name from `[problem.params]`
- standard math functions: `sin`, `cos`, `tan`, `exp`, `log`, `sqrt`,
  `pow`, `abs`, `tanh`, `min`, `max`, ...
- constants: `pi`, `e`

Example:

    [problem.rhs]
    y1 = "y2"
    y2 = "-omega * omega * y1 - damping * y2"

### `[problem.params]`

Optional table of named scalar parameters available in RHS expressions.

    [problem.params]
    omega   = 1.0
    damping = 0.1

### `[solver]`

| Key | Type | Required | Default | Description |
|-----|------|----------|---------|-------------|
| `method` | string | no | `"rk4"` | Integration method |
| `h0` | float | no | `1e-3` | Step size |
| `h_min` | float | no | `1e-12` | Minimum allowed step |
| `h_max` | float | no | `1.0` | Maximum allowed step |
| `max_steps` | int | no | `1000000` | Maximum number of steps |

### `[output]`

| Key | Type | Required | Default | Description |
|-----|------|----------|---------|-------------|
| `path` | string | no | `""` (stdout) | Output file path |
| `format` | string | no | `"csv"` | Output format (currently only `csv`) |

---

## Available methods

| Name | Order | Description |
|------|-------|-------------|
| `euler` | 1 | Explicit Euler |
| `rk2` | 2 | Heun's method (improved Euler) |
| `rk3` | 3 | Classical Kutta's third-order method |
| `rk4` | 4 | Classical fourth-order Runge-Kutta |

All methods use a fixed step size. The last step is truncated to land
exactly on `t_end`.

---

## Library usage

The library core is independent of TOML, expression parsing, and CSV
output. It can be used directly:

    #include <numsol/core/integrator.hpp>
    #include <numsol/core/problem.hpp>
    #include <numsol/methods/rk4.hpp>

    using namespace numsol;

    struct oscillator {
        state operator()(time t, state_view y) const {
            return state{y[1], -y[0]};
        }
    };

    int main() {
        problem<oscillator> p{
            .rhs_   = oscillator{},
            .y0_    = {1.0, 0.0},
            .t0_    = 0.0,
            .t_end_ = 10.0,
        };

        solver_options opts;
        opts.h0_ = 0.001;

        integrator<rk4, oscillator> integ(rk4{}, opts);
        solution sol = integ.run(p);

        // sol.t_  - time points
        // sol.y_  - state vectors
    }

The library consists of two header-only modules:

- `numsol::core` - types, problem, integrator, solution, errors.
- `numsol::methods` - `explicit_rk<Tableau>` and concrete methods.

Everything else (`numsol::app::expr_rhs`, `numsol::app::load_config`,
`numsol::app::write_solution_csv`) belongs to the application layer and
is not required for library use.

---

## Project structure

    numsol/
    ├── include/numsol/
    │   ├── core/          # library: types, problem, integrator
    │   ├── methods/       # library: explicit_rk, euler, rk2, rk3, rk4
    │   ├── expr/          # application: RHS from expressions
    │   ├── io/            # application: TOML config, CSV writer
    │   └── numsol.hpp     # umbrella header
    ├── src/
    │   ├── main.cpp       # entry point
    │   ├── app.{hpp,cpp}  # orchestration
    │   ├── expr/          # .cpp for numsol::app::expr
    │   └── io/            # .cpp for numsol::app::io
    ├── configs/           # example TOML configs
    └── examples/          # (planned) standalone C++ examples

---

## Numerical accuracy

For a smooth ODE, the global error of a method of order `p` scales as
`O(h^p)`. Halving the step size reduces the error by a factor of `2^p`:

| Method | Expected error ratio `err(h/2) / err(h)` |
|--------|------------------------------------------|
| euler  | 2 |
| rk2    | 4 |
| rk3    | 8 |
| rk4    | 16 |

The values above are verified numerically on the harmonic oscillator
with a known analytic solution. To reproduce, run the same problem with
different `h0` and compare the maximum absolute error against `cos(t)`.
