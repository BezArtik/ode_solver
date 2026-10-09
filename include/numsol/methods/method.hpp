/**
 * @file method.hpp
 * @brief Set of available numerical methods.
 */

#pragma once

#include <array>
#include <string_view>
#include <variant>

#include "numsol/methods/adaptive_rk.hpp"
#include "numsol/methods/explicit_rk.hpp"
#include "numsol/methods/rk4_adaptive.hpp"
#include "numsol/methods/tableu.hpp"

namespace numsol {

using euler = explicit_rk<euler_tableau>;
using rk2 = explicit_rk<rk2_tableau>;
using rk3 = explicit_rk<rk3_tableau>;
using rk4 = explicit_rk<rk4_tableau>;
using dopri5 = adaptive_rk<dopri5_tableau>;

/**
 * @brief Variant holding any available numerical method.
 */
using method = std::variant<euler, rk2, rk3, rk4, dopri5, rk4_adaptive>;

/**
 * @brief Entry of @ref method_table.
 */
using method_entry = std::pair<std::string_view, method>;

/**
 * @brief Table of all available methods with their names.
 *
 * Used to look up a method by name at runtime.
 */
// clang-format off
inline constexpr std::array method_table = {
    method_entry{"euler", euler{}}, 
    method_entry{"rk2", rk2{}},
    method_entry{"rk3", rk3{}}, 
    method_entry{"rk4", rk4{}},
    method_entry{"dopri5", dopri5{}},
    method_entry{"rk4_adaptive", rk4_adaptive{}}
};
// clang-format on
}  // namespace numsol
