find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Multimedia OpenGLWidgets)

qt_standard_project_setup()

qt_add_executable(VeCut
    main.cpp
    main_window.cpp
    main_window.h
    media_probe.cpp
    media_probe.h
    video_preview.cpp
    video_preview.h
    audio_preview.cpp
    audio_preview.h
    preview_compositor.cpp
    preview_compositor.h
    preview_shader_library.cpp
    preview_shader_library.h
    gpu_preview_widget.cpp
    gpu_preview_widget.h
)

target_link_libraries(VeCut PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::OpenGLWidgets vecut_core)
target_compile_features(VeCut PRIVATE cxx_std_17)

if(ANDROID)
    set_target_properties(VeCut PROPERTIES
        QT_ANDROID_PACKAGE_NAME "com.mrcreepertamil.vecut"
        QT_ANDROID_APP_NAME "VeCut"
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_APP_ICON "@mipmap/ic_launcher"
        QT_ANDROID_MIN_SDK_VERSION 26
        QT_ANDROID_TARGET_SDK_VERSION 33
    )
endif()

if(IOS)
    # Keep the iOS application target on the same C++/Qt Widgets code path.
    # Qt's iOS platform plugin supplies the native application integration.
    set_target_properties(VeCut PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.mrcreepertamil.vecut"
        MACOSX_BUNDLE_BUNDLE_NAME "VeCut"
    )
endif()
