#include "numsol/expr/expr_parser.hpp"

#include <charconv>
#include <format>
#include <string>
#include <vector>

#include "numsol/core/errors.hpp"

namespace numsol::app {

namespace {

std::size_t parse_index(std::string_view key) noexcept {
    if (key.size() < 2 || key[0] != 'y' || key[1] == '0') return 0;
    auto&& index = std::size_t{};
    auto&& first = key.data() + 1;
    auto&& last = key.data() + key.size();
    auto&& [ptr, ec] = std::from_chars(first, last, index);
    if (ec != std::errc{} || ptr != last) return 0;
    return index;
}

}  // namespace

std::vector<std::string> parse_rhs_expressions(expression_map exprs) {
    if (exprs.empty()) throw invalid_problem_error("rhs expression map is empty");

    auto&& n = exprs.size();
    auto&& result = std::vector<std::string>(n);
    auto&& filled = size_t{};

    for (auto&& [key, value] : exprs) {
        auto&& index = parse_index(key);
        if (index == 0 || index > n) throw invalid_problem_error{std::format("invalid rhs variable name: '{}'", key)};
        result[index - 1] = std::move(value);
        ++filled;
    }

    if (filled != n) throw invalid_problem_error{"rhs variables must form a continuous sequence y1..yN"};

    return result;
}

}  // namespace numsol::app
