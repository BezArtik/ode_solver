# numsol

A command-line utility for numerical integration of systems of ordinary
differential equations (ODEs), with a reusable C++20 library core.

`numsol` solves initial value problems of the form

    dy/dt = f(t, y),   y(t0) = y0

where the right-hand side is given symbolically in a TOML configuration
file. Both fixed-step and adaptive-step Runge-Kutta methods are
available, and the solution is written to CSV (file or stdout).

---

## Table of contents

- [Features](#features)
- [Requirements](#requirements)
- [Building](#building)
- [Usage](#usage)
- [Configuration reference](#configuration-reference)
- [Available methods](#available-methods)
- [Adaptive step-size control](#adaptive-step-size-control)
- [Library usage](#library-usage)
- [Project structure](#project-structure)
- [Numerical accuracy](#numerical-accuracy)
- [License](#license)

---

## Features

- **Symbolic RHS** - define the right-hand side as expressions in a TOML
  file, no recompilation needed.
- **Fixed-step Runge-Kutta methods** - Euler, RK2 (Heun), RK3 (Kutta),
  RK4 (classical).
- **Adaptive-step method** - Dormand-Prince 5(4) with embedded error
  estimate and PI step-size controller.
- **Named parameters** - reusable parameters in RHS expressions.
- **CSV output** - solution to a file or to stdout, full `double`
  precision.
- **Reusable library core** - `numsol::core` and `numsol::methods`
  are independent of TOML, expressions, and CSV.
- **Modern C++23** - concepts, ranges, `std::format`, designated
  initializers.

---

## Requirements

- Compiler with C++23 support 
- CMake 3.24+
- Ninja (recommended) or another CMake generator

Dependencies are fetched automatically via CMake `FetchContent`:

- [toml++](https://github.com/marzer/tomlplusplus) - TOML parsing
- [ExprTk](https://github.com/ArashPartow/exprtk) - expression evaluation
- [Boost.Container](https://github.com/boostorg/container) - `small_vector`
- [GoogleTest](https://github.com/google/googletest) - tests

---

## Building

    cmake --preset clang-release
    cmake --build --preset clang-release

The executable is placed in `out/build/clang-release/src/numsol_app`.

For a debug build with sanitizers:

    cmake --preset clang-debug
    cmake --build --preset clang-debug

Run the tests:

    ctest --preset clang-debug

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

---

## Configuration reference

### `[problem]`

| Key | Type | Required | Default | Description |
|-----|------|----------|---------|-------------|
| `t0` | float | no | `0.0` | Initial time |
| `t_end` | float | **yes** | - | Final time |
| `y0` | array of float | **yes** | - | Initial state vector |

### `[problem.rhs]`

A table mapping variable names `y1, y2, ..., yN` to expression strings.
The names must form a contiguous sequence starting from `y1`. The
number of expressions must equal `y0.size()`.

Available in expressions:

- `y1, y2, ..., yN` - state components
- `t` - current time
- any name from `[problem.params]`
- standard math functions: `sin`, `cos`, `tan`, `exp`, `log`, `sqrt`,
  `pow`, `abs`, `tanh`, `min`, `max`, ...
- constants: `pi`, `e`

### `[problem.params]`

Optional table of named scalar parameters available in RHS expressions.

### `[solver]`

| Key | Type | Required | Default | Description |
|-----|------|----------|---------|-------------|
| `method` | string | no | `"rk4"` | Integration method |
| `h0` | float | no | `1e-3` | Initial step size |
| `h_min` | float | no | `1e-12` | Minimum allowed step |
| `h_max` | float | no | `1.0` | Maximum allowed step |
| `max_steps` | int | no | `1000000` | Maximum number of steps |
| `adaptive` | bool | no | `false` | Enable adaptive step control |
| `rtol` | float | no | `1e-6` | Relative tolerance |
| `atol` | float | no | `1e-9` | Absolute tolerance |
| `safety` | float | no | `0.9` | Safety factor for step control |

The `adaptive`, `rtol`, `atol`, and `safety` keys have no effect with
fixed-step methods.

### `[output]`

| Key | Type | Required | Default | Description |
|-----|------|----------|---------|-------------|
| `path` | string | no | `""` (stdout) | Output file path |
| `format` | string | no | `"csv"` | Output format |

---

## Available methods

| Name | Order | Type | Description |
|------|-------|------|-------------|
| `euler` | 1 | fixed | Explicit Euler |
| `rk2` | 2 | fixed | Heun's method |
| `rk3` | 3 | fixed | Classical Kutta's method |
| `rk4` | 4 | fixed | Classical Runge-Kutta |
| `dopri5` | 5(4) | adaptive | Dormand-Prince 5(4) |

Fixed-step methods use a constant step size. The last step is
truncated to land exactly on `t_end`.

`dopri5` is an adaptive method with an embedded fourth-order error
estimate. It automatically adjusts the step size to satisfy the
requested tolerances.

---

## Adaptive step-size control

Adaptive methods adjust the step size automatically to keep the local
error within the requested tolerances. This is controlled by two
parameters:

- `rtol` - relative tolerance.
- `atol` - absolute tolerance.

At each step, the error is normalized as

    E = sqrt( mean( ((y_next - y_hat) / (atol + rtol * |y|))^2 ) )

where `y_next` is the higher-order solution and `y_hat` is the embedded
lower-order estimate. If `E <= 1`, the step is accepted; otherwise it
is rejected and retried with a smaller step.

The new step size is computed by a PI controller:

    h_new = h * safety * (1 / E)^(1 / p)

where `p` is the order of the method (`p = 5` for `dopri5`) and
`safety` is a safety factor (default `0.9`).

Example using `dopri5`:

    [problem]
    t0    = 0.0
    t_end = 10.0
    y0    = [1.0, 0.0]

    [problem.rhs]
    y1 = "y2"
    y2 = "-y1"

    [solver]
    method   = "dopri5"
    h0       = 0.1
    adaptive = true
    rtol     = 1e-8
    atol     = 1e-10

The value of `h0` is only an initial guess; the method adjusts the
step size on its own.

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
        problem p{
            .rhs_   = oscillator{},
            .y0_    = {1.0, 0.0},
            .t0_    = 0.0,
            .t_end_ = 10.0,
        };

        solver_options opts;
        opts.h0_ = 0.001;

        integrator integ{rk4{}, opts};
        solution sol = integ.run(p);

        // sol.t_  - time points
        // sol.y_  - state vectors
        // sol.stats_ - integration statistics
    }

The library consists of two header-only modules:

- `numsol::core` - types, problem, integrator, solution, errors.
- `numsol::methods` - tableau, explicit_rk, adaptive_rk, concrete methods.

Everything else (`numsol::app::expr_rhs`, `numsol::app::load_config`,
`numsol::app::write_solution_csv`) belongs to the application layer.

---

## Project structure

    numsol/
    ├── include/numsol/
    │   ├── core/          # library: types, problem, integrator
    │   ├── methods/       # library: tableau, explicit_rk, adaptive_rk
    │   ├── expr/          # application: RHS from expressions
    │   ├── io/            # application: TOML config, CSV writer
    │   └── numsol.hpp     # umbrella header
    ├── src/
    │   ├── main.cpp
    │   ├── app.{hpp,cpp}
    │   ├── expr/
    │   └── io/
    ├── tests/
        ├── common/        # shared test helpers
        ├── core/
        ├── methods/
        ├── expr/
        ├── io/
        └── integration/

---

## Numerical accuracy

For a smooth ODE, the global error of a fixed-step method of order `p`
scales as `O(h^p)`. Halving the step size reduces the error by a factor
of `2^p`:

| Method | Expected error ratio `err(h/2) / err(h)` |
|--------|------------------------------------------|
| euler  | 2 |
| rk2    | 4 |
| rk3    | 8 |
| rk4    | 16 |

The values above are verified numerically on the harmonic oscillator
with a known analytic solution.

For adaptive methods, the global error is controlled by the tolerances:

| Tolerance | Expected error |
|-----------|----------------|
| `rtol = 1e-6` | ~`1e-6` |
| `rtol = 1e-9` | ~`1e-9` |

The actual error may be somewhat smaller than the tolerance, since the
controller aims to keep the **local** error bounded, and local errors
partially cancel globally.

---

## License

MIT
