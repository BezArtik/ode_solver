#include "numsol/io/lab_writer.hpp"

#include <array>
#include <format>
#include <fstream>
#include <ostream>
#include <string>

#include "numsol/core/errors.hpp"

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

void write_lab_csv(const solution& sol, std::ostream& out) {
    auto&& t = sol.t_;
    auto&& y = sol.y_;

    if (t.empty() || y.empty()) return;

    auto&& n = t.size();
    auto&& dim = y.front().size();
    auto&& has_coarse = (sol.y_coarse_.size() == n - 1) && n > 0;

    std::string buf;
    buf.reserve(flush_threshold + 1024);

    buf += "i,x";
    for (std::size_t k = 1; k <= dim; ++k) {
        buf += ",v";
        append_integer(buf, k);
    }
    for (std::size_t k = 1; k <= dim; ++k) {
        buf += ",v2_";
        append_integer(buf, k);
    }
    for (std::size_t k = 1; k <= dim; ++k) {
        buf += ",d_";
        append_integer(buf, k);
    }
    buf += ",LEE,h,C1,C2\n";
    out.write(buf.data(), static_cast<std::streamsize>(buf.size()));
    buf.clear();

    for (std::size_t i = 0; i < n; ++i) {
        auto&& fine = y[i];
        auto&& has_data = has_coarse && i > 0;

        append_integer(buf, i);
        buf += ',';
        append_number(buf, t[i]);

        for (std::size_t k = 0; k < dim; ++k) {
            buf += ',';
            append_number(buf, has_data ? sol.y_coarse_[i - 1][k] : fine[k]);
        }

        for (std::size_t k = 0; k < dim; ++k) {
            buf += ',';
            append_number(buf, fine[k]);
        }

        for (std::size_t k = 0; k < dim; ++k) {
            buf += ',';
            append_number(buf, has_data ? (sol.y_coarse_[i - 1][k] - fine[k]) : 0.0);
        }

        if (has_data) {
            buf += ',';
            append_number(buf, sol.lee_[i - 1]);
            buf += ',';
            append_number(buf, sol.h_[i - 1]);
            buf += ',';
            append_integer(buf, sol.c1_[i - 1]);
            buf += ',';
            append_integer(buf, sol.c2_[i - 1]);
        } else {
            buf += ",0,0,0,0";
        }

        buf += '\n';

        if (buf.size() >= flush_threshold) {
            out.write(buf.data(), static_cast<std::streamsize>(buf.size()));
            buf.clear();
        }
    }

    if (!buf.empty()) out.write(buf.data(), static_cast<std::streamsize>(buf.size()));
}

void write_lab_csv(const solution& sol, const std::string& path) {
    auto&& file = std::ofstream{path};
    if (!file.is_open()) throw invalid_problem_error{std::format("failed to open output file: '{}'", path)};
    write_lab_csv(sol, file);
}

}  // namespace numsol::app
