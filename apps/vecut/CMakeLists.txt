find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

qt_standard_project_setup()

qt_add_executable(VeCut
    main.cpp
    main_window.cpp
    main_window.h
)

target_link_libraries(VeCut PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets vecut_core)
target_compile_features(VeCut PRIVATE cxx_std_17)

if(ANDROID)
    set_target_properties(VeCut PROPERTIES
        QT_ANDROID_PACKAGE_NAME "com.mrcreepertamil.vecut"
        QT_ANDROID_APP_NAME "VeCut"
        QT_ANDROID_MIN_SDK_VERSION 24
        QT_ANDROID_TARGET_SDK_VERSION 35
    )
endif()
