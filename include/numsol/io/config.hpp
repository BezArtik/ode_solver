/**
 * @file config.hpp
 * @brief TOML configuration loading.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "numsol/core/options.hpp"
#include "numsol/core/types.hpp"

namespace numsol::app {

/**
 * @brief Configuration of a single solve run.
 */
struct config {
    /// Initial state.
    state y0_;

    /// Initial time.
    time t0_ = 0.0;

    /// Final time.
    time t_end_ = 0.0;

    /// Expressions for the right-hand side, ordered as @c y1..yN.
    std::vector<std::string> rhs_expressions_;

    /// Named parameters available in RHS expressions.
    std::unordered_map<std::string, scalar> params_;

    /// Name of the numerical method.
    std::string method_ = "rk4";

    /// Numerical options.
    solver_options opts_;

    /// Output file path; empty means stdout.
    std::string output_path_;

    /// Output format (currently only @c "csv").
    std::string output_format_ = "csv";
};

/**
 * @brief Loads a configuration from a TOML file.
 *
 * @param path Path to the TOML file.
 * @return     Parsed configuration.
 *
 * @throws numsol::invalid_problem_error if the file cannot be read,
 *         contains a syntax error, is missing a required field, has a
 *         field of the wrong type, or has mismatched @c y0 and RHS
 *         dimensions.
 */
[[nodiscard]] config load_config(std::string_view path);

}  // namespace numsol::app
