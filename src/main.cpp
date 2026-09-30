#include <exception>
#include <print>

#include "app.hpp"
#include "numsol/core/errors.hpp"
#include "numsol/io/config.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::print(stderr, "usage: {} <config.toml>\n", argv[0]);
        return 1;
    }

    try {
        auto&& cfg = numsol::app::load_config(argv[1]);
        numsol::app::run(cfg);
        return 0;
    } catch (const numsol::solver_error& e) {
        std::print(stderr, "error: {}\n", e.what());
        return 1;
    } catch (const std::exception& e) {
        std::print(stderr, "unexpected error: {}\n", e.what());
        return 2;
    }
}
