cmake_minimum_required(VERSION 3.22.1)
project(vecut_engine LANGUAGES CXX)

add_library(vecut_engine SHARED
    native-lib.cpp
    timeline.cpp
)

target_compile_features(vecut_engine PRIVATE cxx_std_17)
target_compile_options(vecut_engine PRIVATE -Wall -Wextra -Werror)
