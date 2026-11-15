find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(VeCut
    main.cpp
    main_window.cpp
    main_window.h
)

target_link_libraries(VeCut PRIVATE Qt6::Widgets vecut_core)
target_compile_features(VeCut PRIVATE cxx_std_17)
