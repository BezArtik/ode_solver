#include "app.hpp"

#include "numsol/core/errors.hpp"
#include "numsol/core/integrator.hpp"
#include "numsol/core/problem.hpp"
#include "numsol/expr/expr_rhs.hpp"
#include "numsol/io/lab_writer.hpp"
#include "numsol/io/solution_writer.hpp"
#include "numsol/methods/method.hpp"

#include <algorithm>
#include <format>
#include <print>
#include <utility>
#include <variant>

namespace numsol::app {

void run(const config& cfg) {
    auto&& rhs = expr_rhs{cfg.rhs_expressions_, cfg.params_};

    auto&& p = problem{std::move(rhs), cfg.y0_, cfg.t0_, cfg.t_end_};

    auto&& it = std::ranges::find(method_table, cfg.method_, &method_entry::first);
    if (it == method_table.end()) throw invalid_problem_error{std::format("unknown method: '{}'", cfg.method_)};

    auto&& sol = std::visit(
        [&](auto concrete_method) {
            auto&& integ = integrator{std::move(concrete_method), cfg.opts_};
            return integ.run(p);
        },
        std::move(it->second));

    cfg.output_format_ == "lab" ? write_lab_csv(sol, cfg.output_path_) : write_solution_csv(sol, cfg.output_path_);
    // clang-format off
    std::print(stderr,
               "n = {}\n" 
               "b - x_n = {}\n" 
               "max |LEE| = {}\n" 
               "total halvings = {}\n" 
               "total doublings = {}\n" 
               "max h = {} at x = {}\n" 
               "min h = {} at x = {}\n",
               sol.stats_.steps_, cfg.t_end_ - sol.t_.back(), sol.stats_.max_lee_, sol.stats_.halvings_,
               sol.stats_.doublings_, sol.stats_.max_h_, sol.stats_.max_h_at_, sol.stats_.min_h_, sol.stats_.min_h_at_);
    // clang-format on
}

}  // namespace numsol::app
