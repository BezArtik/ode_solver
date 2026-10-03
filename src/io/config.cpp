#include "numsol/io/config.hpp"

#include <format>
#include <string>
#include <string_view>
#include <toml++/toml.hpp>

#include "numsol/core/errors.hpp"
#include "numsol/expr/expr_parser.hpp"

namespace numsol::app {

namespace {

[[noreturn]] void missing_field(std::string_view section, std::string_view field) {
    throw invalid_problem_error{std::format("config: missin required field '{}.{}'", section, field)};
}

[[noreturn]] void wrong_type(std::string_view section, std::string_view field, std::string_view expected) {
    throw invalid_problem_error{std::format("config: field '{}.{}' must be {}", section, field, expected)};
}

auto get_time(const toml::table& tbl, std::string_view section, std::string_view field, time default_value) {
    auto&& node = tbl[field].as_floating_point();
    if (!node) {
        if (tbl[field]) wrong_type(section, field, "a floating-point number");
        return default_value;
    }
    return node->get();
}

auto get_required_time(const toml::table& tbl, std::string_view section, std::string_view field) {
    auto&& node = tbl[field].as_floating_point();
    if (!node) {
        if (tbl[field]) wrong_type(section, field, "a floating-point number");
        missing_field(section, field);
    }
    return node->get();
}

auto get_size(const toml::table& tbl, std::string_view section, std::string_view field, std::size_t default_value) {
    auto&& node = tbl[field].as_integer();
    if (!node) {
        if (tbl[field]) wrong_type(section, field, "an integer");
        return default_value;
    }
    auto&& value = node->get();
    if (value < 0) {
        throw invalid_problem_error{std::format("config: field '{}.{}' must be non-negative", section, field)};
    }
    return static_cast<std::size_t>(value);
}

auto get_string(const toml::table& tbl, std::string_view section, std::string_view field, std::string default_value) {
    auto&& node = tbl[field].as_string();
    if (!node) {
        if (tbl[field]) wrong_type(section, field, "a string");
        return default_value;
    }
    return node->get();
}

auto parse_y0(const toml::table& problem) {
    auto&& arr = problem["y0"].as_array();
    if (!arr) {
        if (problem["y0"]) wrong_type("problem", "y0", "an array of numbers");
        missing_field("problem", "y0");
    }

    auto&& y0 = state{};
    y0.reserve(arr->size());
    for (auto&& elem : *arr) {
        auto&& num = elem.as_floating_point();
        if (!num) throw invalid_problem_error{"config: 'problem.y0' must contain only numbers"};
        y0.push_back(num->get());
    }
    return y0;
}

auto parse_rhs(const toml::table& problem) {
    auto&& tbl = problem["rhs"].as_table();
    if (!tbl) {
        if (problem["rhs"]) wrong_type("problem", "rhs", "a table of expressions");
        missing_field("problem", "rhs");
    }

    auto&& result = expression_map{};
    result.reserve(tbl->size());
    for (auto&& [key, value] : *tbl) {
        auto&& str = value.as_string();
        if (!str) throw invalid_problem_error{std::format("config: 'problem.rhs.{}' must be a string", key.str())};
        result.emplace(key.str(), str->get());
    }
    return result;
}

auto parse_params(const toml::table& problem) {
    auto&& tbl = problem["params"].as_table();
    if (!tbl) return std::unordered_map<std::string, scalar>{};

    auto&& result = std::unordered_map<std::string, scalar>{};
    result.reserve(tbl->size());
    for (auto&& [key, value] : *tbl) {
        auto&& num = value.as_floating_point();
        if (!num) throw invalid_problem_error{std::format("config: 'problem.params.{}' must be a number", key.str())};
        result.emplace(key.str(), num->get());
    }
    return result;
}

auto parse_solver(const toml::table& tbl) {
    auto&& opts = solver_options{};
    opts.h0_ = get_time(tbl, "solver", "h0", opts.h0_);
    opts.h_min_ = get_time(tbl, "solver", "h_min", opts.h_min_);
    opts.h_max_ = get_time(tbl, "solver", "h_max", opts.h_max_);
    opts.max_steps_ = get_size(tbl, "solver", "max_steps", opts.max_steps_);
    return opts;
}

}  // namespace

config load_config(std::string_view path) {
    auto&& tbl = toml::table{};
    try {
        tbl = toml::parse_file(path);
    } catch (const toml::parse_error& e) {
        throw invalid_problem_error{std::format("config: failed to parse '{}': {}", path, e.description())};
    }

    auto&& problem = tbl["problem"].as_table();
    if (!problem) missing_field("problem", "(section)");

    auto&& solver = tbl["solver"].as_table();

    auto&& cfg = config{};

    cfg.t0_ = get_time(*problem, "problem", "t0", cfg.t0_);
    cfg.t_end_ = get_required_time(*problem, "problem", "t_end");
    cfg.y0_ = parse_y0(*problem);
    cfg.params_ = parse_params(*problem);

    auto&& raw_rhs = parse_rhs(*problem);
    cfg.rhs_expressions_ = parse_rhs_expressions(raw_rhs);

    if (cfg.y0_.size() != cfg.rhs_expressions_.size())
        throw invalid_problem_error{std::format("config: size mismatch: y0 has {} components, but rhs defines {}",
                                                cfg.y0_.size(), cfg.rhs_expressions_.size())};

    if (solver) {
        cfg.method_ = get_string(*solver, "solver", "method", cfg.method_);
        cfg.opts_ = parse_solver(*solver);
    }

    auto&& output = tbl["output"].as_table();
    if (output) {
        cfg.output_path_ = get_string(*output, "output", "path", "");
        cfg.output_format_ = get_string(*output, "output", "format", "csv");
    }

    return cfg;
}

}  // namespace numsol::app
