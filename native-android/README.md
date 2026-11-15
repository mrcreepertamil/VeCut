# VeCut Android transition prototype

This directory contains the earlier Android-native prototype while the project moves to one C++17 application for Windows, Android, Linux, macOS and iOS.

The new shared application is being built from the repository-root CMake project:

- `engine/` — platform-independent C++17 project and timeline model.
- `apps/vecut/` — Qt 6 Widgets UI shell written in C++17.
- `CMakeLists.txt` — shared build entry point for the C++ application.

The Android prototype still contains Java activities, Android-specific media classes and a JNI bridge. These are transitional code, not the intended final Android editor. The Android CMake target now compiles the shared `engine/src/project.cpp` source so the project model can be reused while the Android integration is migrated.

## Existing prototype capabilities

The old prototype includes an Android editor-style workspace, media document picker, MediaCodec video preview through OpenGL ES, basic timeline metadata, clip trim/split/transition settings and standalone audio preview. These features are not yet all connected to a shared cross-platform playback and export pipeline.

## Migration rules

- Keep the shared editing model and application UI in C++17.
- Keep operating-system-specific integration behind narrow backend interfaces.
- Do not add Flutter or Dart to the new application.
- Do not delete this prototype or `../.oldproto` until replacement features have been ported and verified.
- Remove the remaining Java editor implementation only as the C++ Android application reaches feature parity.

The C++ Qt app is an early shell. It is not yet a working replacement for the Android prototype and has not yet been built and tested on all five target platforms.
