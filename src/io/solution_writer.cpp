#include "numsol/io/solution_writer.hpp"

#include "numsol/core/errors.hpp"

#include <format>
#include <fstream>
#include <ostream>
#include <string>

namespace numsol::app {

namespace {

constexpr std::size_t flush_threshold = 64 * 1024;

void append_number(std::string& buf, double value) {
    std::array<char, 32> tmp;
    auto [ptr, _] = std::to_chars(tmp.begin(), tmp.end(), value);
    buf.append(tmp.begin(), ptr);
}

void append_integer(std::string& buf, std::size_t value) {
    std::array<char, 24> tmp;
    auto [ptr, _] = std::to_chars(tmp.begin(), tmp.end(), value);
    buf.append(tmp.begin(), ptr);
}

}  // namespace

void write_solution_csv(const solution& sol, std::ostream& out) {
    auto&& t = sol.t_;
    auto&& y = sol.y_;

    if (t.empty() || y.empty()) return;

    auto&& dim = y.front().size();

    std::string buf;
    buf.reserve(flush_threshold + 1024);

    buf += 't';
    for (std::size_t i = 1; i <= dim; ++i) {
        buf += ",y";
        append_integer(buf, i);
    }
    buf += '\n';
    out.write(buf.data(), static_cast<std::streamsize>(buf.size()));
    buf.clear();

    for (std::size_t k = 0; k < t.size(); ++k) {
        append_number(buf, t[k]);
        for (auto value : y[k]) {
            buf += ',';
            append_number(buf, value);
        }
        buf += '\n';

        if (buf.size() >= flush_threshold) {
            out.write(buf.data(), static_cast<std::streamsize>(buf.size()));
            buf.clear();
        }
    }

    if (!buf.empty()) out.write(buf.data(), static_cast<std::streamsize>(buf.size()));
}

void write_solution_csv(const solution& sol, const std::string& path) {
    auto&& file = std::ofstream{path};
    if (!file.is_open()) throw invalid_problem_error{std::format("failed to open file: '{}'", path)};
    write_solution_csv(sol, file);
}

}  // namespace numsol::app
