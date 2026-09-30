/**
 * @file solution_writer.hpp
 * @brief CSV output of a numerical solution.
 */

#pragma once

#include <iosfwd>
#include <string>

#include "numsol/core/solution.hpp"

namespace numsol::app {

/**
 * @brief Writes a solution to a stream in CSV format.
 *
 * Format: one header line @c t,y1,y2,...,yN followed by one line per
 * time point. Values are written with full @c double precision.
 *
 * @param sol Solution to write.
 * @param out Output stream.
 */
void write_solution_csv(const solution& sol, std::ostream& out);

/**
 * @brief Writes a solution to a CSV file.
 *
 * @param sol  Solution to write.
 * @param path Path to the output file.
 *
 * @throws numsol::invalid_problem_error if the file cannot be opened.
 */
void write_solution_csv(const solution& sol, const std::string& path);

}  // namespace numsol::app
