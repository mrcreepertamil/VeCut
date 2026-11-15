cmake_minimum_required(VERSION 3.22)
project(VeCut LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
option(VECUT_BUILD_DESKTOP "Build the Qt desktop application" ON)

add_subdirectory(engine)

if(VECUT_BUILD_DESKTOP)
    add_subdirectory(apps/vecut)
endif()
