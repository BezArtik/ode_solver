add_executable(numsol_tests
    core/test_integrator.cpp
    methods/test_fixed.cpp
    methods/test_adaptive.cpp
    expr/test_expr_parser.cpp
    expr/test_expr_rhs.cpp
    io/test_config.cpp
    io/test_solution_writer.cpp
    integration/test_end_to_end.cpp
)

target_link_libraries(numsol_tests PRIVATE
    numsol::numsol
    GTest::gtest_main
)

target_compile_features(numsol_tests PRIVATE cxx_std_23)

target_include_directories(numsol_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include(GoogleTest)
gtest_discover_tests(numsol_tests)
