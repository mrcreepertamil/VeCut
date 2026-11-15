add_executable(vecut_project_tests project_tests.cpp)
target_link_libraries(vecut_project_tests PRIVATE vecut_core)
target_compile_features(vecut_project_tests PRIVATE cxx_std_17)
add_test(NAME vecut_project_tests COMMAND vecut_project_tests)

# Keep assert-based checks active in Release configurations as well.
if(MSVC)
    target_compile_options(vecut_project_tests PRIVATE /UNDEBUG)
else()
    target_compile_options(vecut_project_tests PRIVATE -UNDEBUG)
endif()
