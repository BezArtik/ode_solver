#include "numsol/expr/expr_rhs.hpp"

#include "exprtk.hpp"
#include "numsol/core/errors.hpp"

#include <algorithm>
#include <format>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace numsol::app {

struct expr_rhs::impl {
    std::map<std::string, scalar> params_;

    scalar t_var_ = 0.0;
    state y_vars_;

    exprtk::symbol_table<scalar> symtab_;
    std::vector<exprtk::expression<scalar>> exprs_;

    impl(std::vector<std::string> expressions, std::unordered_map<std::string, scalar> params)
        : y_vars_(expressions.size()) {
        params_.insert(std::make_move_iterator(params.begin()), std::make_move_iterator(params.end()));

        symtab_.add_variable("t", t_var_);

        for (std::size_t i = 0; i < expressions.size(); ++i)
            symtab_.add_variable(std::format("y{}", i + 1), y_vars_[i]);

        for (auto&& [name, value] : params_) symtab_.add_variable(name, value);

        exprs_.resize(expressions.size());
        exprtk::parser<scalar> parser;

        for (std::size_t i = 0; i < expressions.size(); ++i) {
            exprs_[i].register_symbol_table(symtab_);
            if (!parser.compile(expressions[i], exprs_[i])) {
                throw invalid_problem_error{
                    std::format("failed to compile rhs expression for y{} : {}", i + 1, parser.error())};
            }
        }
    }
};

expr_rhs::expr_rhs(std::vector<std::string> expressions, std::unordered_map<std::string, scalar> params)
    : impl_(std::make_unique<impl>(std::move(expressions), std::move(params))) {}

expr_rhs::~expr_rhs() = default;
expr_rhs::expr_rhs(expr_rhs&&) noexcept = default;
expr_rhs& expr_rhs::operator=(expr_rhs&&) noexcept = default;

state expr_rhs::operator()(time t, state_view y) const {
    auto&& p = *impl_;

    if (y.size() != p.y_vars_.size())
        throw invalid_problem_error{
            std::format("rhs: state size mismatch (expected {}, got {})", p.y_vars_.size(), y.size())};

    p.t_var_ = t;
    std::ranges::copy(y, p.y_vars_.begin());

    auto&& result = state(p.exprs_.size());
    std::ranges::transform(p.exprs_, result.begin(), [](auto&& e) { return e.value(); });

    return result;
}

}  // namespace numsol::app
