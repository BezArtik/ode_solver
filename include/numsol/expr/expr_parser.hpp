/**
 * @file expr_parser.hpp
 * @brief Parsing of RHS expression dictionaries.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace numsol::app {

/**
 * @brief Dictionary mapping variable names to expression strings.
 */
using expression_map = std::unordered_map<std::string, std::string>;

/**
 * @brief Converts a name-to-expression dictionary into an ordered list.
 *
 * Keys must form a contiguous sequence @c y1, @c y2, ..., @c yN.
 *
 * @param exprs Dictionary of expressions.
 * @return      Expressions ordered by variable index.
 *
 * @throws numsol::invalid_problem_error if the dictionary is empty,
 *         a key is malformed, or the sequence is not contiguous.
 */
[[nodiscard]] std::vector<std::string> parse_rhs_expressions(expression_map exprs);

}  // namespace numsol::app
