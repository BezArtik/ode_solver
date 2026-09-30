#include "numsol/expr/expr_parser.hpp"

#include <algorithm>
#include <charconv>
#include <format>
#include <ranges>
#include <string>
#include <vector>

#include "numsol/core/errors.hpp"

namespace numsol::app {

namespace {

std::size_t parse_index(std::string_view key) noexcept {
    if (key.size() < 2 || key[0] != 'y') return 0;
    auto&& index = std::size_t{};
    auto&& first = key.data() + 1;
    auto&& last = key.data() + key.size();
    auto&& [ptr, ec] = std::from_chars(first, last, index);
    if (ec != std::errc{} || ptr != last || index == 0) return 0;
    return index;
}

}  // namespace

parsed_rhs parse_rhs_expressions(const std::unordered_map<std::string, std::string>& exprs) {
    if (exprs.empty()) throw invalid_problem_error{"rhs expression map is empty"};

    struct entry {
        std::size_t index;
        const std::string* expr;
    };

    std::vector<entry> entries;
    entries.reserve(exprs.size());

    for (auto&& [key, value] : exprs) {
        auto&& index = parse_index(key);
        if (index == 0) throw invalid_problem_error{std::format("invalid rhs variable name: '{}'", key)};
        entries.emplace_back(index, &value);
    }

    std::ranges::sort(entries, {}, &entry::index);

    auto&& indices = entries | std::views::transform(&entry::index);

    auto&& continuous = std::ranges::equal(indices, std::views::iota(std::size_t{1}, entries.size() + 1));
    if (!continuous) throw invalid_problem_error{"rhs variables must form a continuous sequence y1..yN"};

    parsed_rhs result;
    result.expressions_.reserve(entries.size());
    std::ranges::transform(entries, std::back_inserter(result.expressions_), [](auto&& e) { return *e.expr; });

    return result;
}

}  // namespace numsol::app
