add_library(vecut_core STATIC
    src/project.cpp
)
target_include_directories(vecut_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(vecut_core PUBLIC cxx_std_17)
set_target_properties(vecut_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    target_compile_options(vecut_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(vecut_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
