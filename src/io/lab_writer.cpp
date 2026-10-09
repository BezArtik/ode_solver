#include "numsol/io/lab_writer.hpp"

#include <format>
#include <fstream>
#include <ostream>
#include <string>

#include "numsol/core/errors.hpp"

namespace numsol::app {

void write_lab_csv(const solution& sol, std::ostream& out) {
    auto&& t = sol.t_;
    auto&& y = sol.y_;

    if (t.empty() || y.empty()) return;

    auto&& n = t.size();
    auto&& dim = y.front().size();

    auto&& has_coarse = (sol.y_coarse_.size() == n - 1) && n > 0;

    out << "i,x";
    for (std::size_t k = 1; k <= dim; ++k) out << ",v" << k;
    for (std::size_t k = 1; k <= dim; ++k) out << ",v2_" << k;
    for (std::size_t k = 1; k <= dim; ++k) out << ",d_" << k;
    out << ",LEE,h,C1,C2\n";

    for (std::size_t i = 0; i < n; ++i) {
        auto&& fine = y[i];

        // i, x.
        // out << i << ',' << fmt(t[i]);
        out << std::format("{},{:.17g}", i, t[i]);

        // v_k — coarse.
        for (std::size_t k = 0; k < dim; ++k) {
            // out << ',';
            // if (has_coarse && i > 0) {
            //     out << fmt(sol.y_coarse_[i - 1][k]);
            // } else {
            //     out << fmt(fine[k]);
            // }
            out << std::format(",{:.17g}", has_coarse && i > 0 ? sol.y_coarse_[i - 1][k] : fine[k]);
        }

        // v2_k — fine.
        for (std::size_t k = 0; k < dim; ++k) {
            // out << ',' << fmt(fine[k]);
            out << std::format(",{:.17g}", fine[k]);
        }

        // d_k = v_k - v2_k.
        for (std::size_t k = 0; k < dim; ++k) {
            // out << ',';
            // if (has_coarse && i > 0) {
            //     out << fmt(sol.y_coarse_[i - 1][k] - fine[k]);
            // } else {
            //     out << fmt(0.0);
            // }
            out << std::format(",{:.17g}", has_coarse && i > 0 ? (sol.y_coarse_[i - 1][k] - fine[k]) : 0);
        }

        // LEE, h, C1, C2.
        if (has_coarse && i > 0) {
            // out << ',' << fmt(sol.lee_[i - 1]) << ',' << fmt(sol.h_[i - 1]) << ',' << sol.c1_[i - 1] << ','
            //     << sol.c2_[i - 1];
            out << std::format(",{:.17g},{:.17g},{},{}", sol.lee_[i - 1], sol.h_[i - 1], sol.c1_[i - 1],
                               sol.c2_[i - 1]);
        } else {
            // out << ',' << fmt(0.0) << ',' << fmt(0.0) << ',' << 0 << ',' << 0;
            out << std::format(",{:.17g},{:.17g},{},{}", 0.0, 0.0, 0, 0);
        }

        out << '\n';
    }
}

void write_lab_csv(const solution& sol, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) throw invalid_problem_error{std::format("failed to open output file: '{}'", path)};

    write_lab_csv(sol, file);
}

}  // namespace numsol::app
