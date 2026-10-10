add_executable(numsol_app
    main.cpp
    app/app.cpp
)

target_link_libraries(numsol_app PRIVATE
    numsol::numsol
)

target_compile_features(numsol_app PRIVATE cxx_std_23)
