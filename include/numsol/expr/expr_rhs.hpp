/**
 * @file expr_rhs.hpp
 * @brief Right-hand side defined by symbolic expressions.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "numsol/core/types.hpp"

namespace numsol::app {

/**
 * @brief Right-hand side evaluated from expression strings.
 *
 * Holds one expression per state component (ordered as @c y1, @c y2,
 * ..., @c yN) and a set of named parameters available inside those
 * expressions.
 *
 * Names available in expressions:
 * - @c y1, @c y2, ..., @c yN — state components,
 * - @c t — current time,
 * - any name from the parameter map.
 *
 * Satisfies @ref numsol::rhs and can be used directly with
 * @ref numsol::integrator.
 *
 * @note Non-copyable; movable.
 */
class expr_rhs {
public:
    /**
     * @brief Constructs an RHS from expression strings.
     *
     * @param expressions One expression per state component, in order
     *                    @c y1, @c y2, ..., @c yN.
     * @param params      Named parameters available in expressions.
     *
     * @throws numsol::invalid_problem_error if any expression fails to
     *         compile.
     */
    expr_rhs(std::vector<std::string> expressions, std::unordered_map<std::string, scalar> params = {});
    ~expr_rhs();

    /// Move constructor
    expr_rhs(expr_rhs&&) noexcept;

    /// Move assignment
    expr_rhs& operator=(expr_rhs&&) noexcept;

    expr_rhs(const expr_rhs&) = delete;
    expr_rhs& operator=(const expr_rhs&) = delete;

    /**
     * @brief Evaluates the right-hand side.
     *
     * @param t Current time.
     * @param y Current state.
     * @return  Derivatives of all components.
     *
     * @throws numsol::invalid_problem_error if @p y has the wrong size.
     */
    [[nodiscard]] state operator()(time t, state_view y) const;

private:
    struct impl;
    std::unique_ptr<impl> impl_;
};

}  // namespace numsol::app
