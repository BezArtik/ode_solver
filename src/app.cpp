#include "app.hpp"

#include <algorithm>
#include <format>
#include <utility>
#include <variant>

#include "numsol/core/errors.hpp"
#include "numsol/core/integrator.hpp"
#include "numsol/core/problem.hpp"
#include "numsol/expr/expr_rhs.hpp"
#include "numsol/io/lab_writer.hpp"
#include "numsol/io/solution_writer.hpp"
#include "numsol/methods/method.hpp"

namespace numsol::app {

void run(const config& cfg) {
    auto&& rhs = expr_rhs{cfg.rhs_expressions_, cfg.params_};

    auto&& p = problem{std::move(rhs), cfg.y0_, cfg.t0_, cfg.t_end_};

    auto&& it = std::ranges::find(method_table, cfg.method_, &method_entry::first);
    if (it == method_table.end()) throw invalid_problem_error{std::format("unknown method: '{}'", cfg.method_)};

    auto&& sol = std::visit(
        [&](auto concrete_method) {
            integrator integ(std::move(concrete_method), cfg.opts_);
            return integ.run(p);
        },
        std::move(it->second));

    if (cfg.output_format_ == "lab") {
        write_lab_csv(sol, cfg.output_path_);
    } else {
        write_solution_csv(sol, cfg.output_path_);
    }
}

}  // namespace numsol::app
