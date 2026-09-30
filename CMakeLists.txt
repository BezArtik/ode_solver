cmake_minimum_required(VERSION 4.1...4.3)

project(numsol DESCRIPTION "Numerical ODE solvers library" LANGUAGES CXX)

include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    exprtk
    GIT_REPOSITORY https://github.com/ArashPartow/exprtk.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(tomlplusplus exprtk)

add_library(exprtk INTERFACE)
target_include_directories(exprtk INTERFACE
    ${exprtk_SOURCE_DIR}
)
add_library(exprtk::exprtk ALIAS exprtk)

add_library(numsol_core INTERFACE)
target_include_directories(numsol_core INTERFACE include)
target_compile_features(numsol_core INTERFACE cxx_std_23)
add_library(numsol::core ALIAS numsol_core)

add_library(numsol_methods INTERFACE)
target_link_libraries(numsol_methods INTERFACE numsol::core)
add_library(numsol::methods ALIAS numsol_methods)

add_library(numsol_expr STATIC
    src/expr/expr_rhs.cpp
    src/expr/expr_parser.cpp
)
target_include_directories(numsol_expr PUBLIC include)
target_link_libraries(numsol_expr PUBLIC
    numsol::core
    exprtk::exprtk
)
add_library(numsol::expr ALIAS numsol_expr)

add_library(numsol_io STATIC
    src/io/config.cpp
    src/io/solution_writer.cpp
)
target_include_directories(numsol_io PUBLIC include)
target_link_libraries(numsol_io PUBLIC
    numsol::core
    numsol::expr
    tomlplusplus::tomlplusplus
)
add_library(numsol::io ALIAS numsol_io)

add_library(numsol INTERFACE)
target_link_libraries(numsol INTERFACE
    numsol::core
    numsol::methods
    numsol::expr
    numsol::io
)
add_library(numsol::numsol ALIAS numsol)

# --- Подпроекты ---
add_subdirectory(src)
