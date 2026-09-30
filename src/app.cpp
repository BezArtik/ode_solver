#include "app.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include "numsol/core/errors.hpp"
#include "numsol/core/integrator.hpp"
#include "numsol/core/problem.hpp"
#include "numsol/core/solution.hpp"
#include "numsol/expr/expr_rhs.hpp"
#include "numsol/io/solution_writer.hpp"
#include "numsol/methods/method.hpp"

namespace numsol::app {

void run(const config& cfg) {
    auto&& rhs = expr_rhs{cfg.rhs_expressions_, cfg.params_};

    auto&& p = problem<expr_rhs>{
        .rhs_ = std::move(rhs),
        .y0_ = cfg.y0_,
        .t0_ = cfg.t0_,
        .t_end_ = cfg.t_end_,
    };

    auto&& it = std::ranges::find(method_table, cfg.method_, &method_entry::first);
    if (it == method_table.end()) throw invalid_problem_error{std::format("unknown method: '{}'", cfg.method_)};

    solution sol = std::visit(
        [&](auto&& concrete_method) {
            integrator<std::decay_t<decltype(concrete_method)>, expr_rhs> integ(concrete_method, cfg.opts_);
            return integ.run(p);
        },
        it->second);

    if (cfg.output_path_.empty()) {
        write_solution_csv(sol, std::cout);
    } else {
        write_solution_csv(sol, cfg.output_path_);
    }
}

}  // namespace numsol::app
