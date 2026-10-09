/**
 * @file lab_writer.hpp
 * @brief Extended CSV output for laboratory work.
 */

#pragma once

#include <iosfwd>
#include <string>

#include "numsol/core/solution.hpp"

namespace numsol::app {

/**
 * @brief Writes a solution in the extended CSV format.
 *
 * Columns:
 *   - @c i       - step index (0 = initial point)
 *   - @c x       - time
 *   - @c v_k     - coarse solution (full step), component @c k
 *   - @c v2_k    - fine solution (two half-steps), component @c k
 *   - @c d_k     - difference @c v_k - @c v2_k
 *   - @c LEE     - local error estimate
 *   - @c h       - step size used
 *   - @c C1      - cumulative halvings
 *   - @c C2      - cumulative doublings
 *
 * The @c v_k, @c v2_k, and @c d_k columns are populated only for
 * methods that produce double-step data (such as
 * @ref numsol::rk4_adaptive). For other methods they are written
 * as zeros.
 *
 * @param sol Solution to write.
 * @param out Output stream.
 */
void write_lab_csv(const solution& sol, std::ostream& out);

/**
 * @brief Writes the extended CSV to a file.
 *
 * @throws numsol::invalid_problem_error if the file cannot be opened.
 */
void write_lab_csv(const solution& sol, const std::string& path);

}  // namespace numsol::app
