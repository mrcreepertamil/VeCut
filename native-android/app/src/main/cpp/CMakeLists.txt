cmake_minimum_required(VERSION 3.22.1)
project(vecut_engine LANGUAGES CXX)

set(VECUT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../")

add_library(vecut_engine SHARED
    native-lib.cpp
    timeline.cpp
    ${VECUT_ROOT}/engine/src/project.cpp
)

target_include_directories(vecut_engine PRIVATE
    ${VECUT_ROOT}/engine/include
)

target_compile_features(vecut_engine PRIVATE cxx_std_17)
target_compile_options(vecut_engine PRIVATE -Wall -Wextra -Werror)
