# VeCut

**VeCut is an in-development cross-platform video editor.** The long-term target is one C++17 application and one shared editor experience on Windows, Android, Linux, macOS and iOS.

The project is moving away from Flutter/Dart and separate platform-language editor implementations. The application UI and editing engine are being built in C++17. Operating-system APIs may still be used behind narrow integration layers for media codecs, files, windowing, packaging and device capabilities; those layers are not separate editor implementations.

## Technology direction

- **C++17** — application UI, project model, timeline, editing operations, effects, transitions, audio graph and render pipeline.
- **Qt 6 Widgets** — the initial all-C++ cross-platform UI framework. The same UI target is intended to be configured for Windows, Android, Linux, macOS and iOS using the relevant Qt kits/toolchains.
- **CMake** — shared build configuration and reusable engine target.
- **Platform media backends** — selected behind C++ interfaces as the engine grows. The existing Android MediaCodec/OpenGL ES preview is transitional and is not yet the cross-platform renderer.
- **JavaScript plugins** — optional future third-party plugin scripting only; not used to implement the editor or its UI.

## New C++ foundation

- `engine/` contains the platform-independent C++17 project model.
- The model currently represents media assets, video/audio/overlay tracks, clips, source trim ranges, incoming transitions, clip volume, splitting, clip ordering and project validation.
- `apps/vecut/` contains the initial Qt Widgets application shell, written entirely in C++.
- The root `CMakeLists.txt` builds the shared engine and Qt application.

This is an architectural foundation, not a finished video editor. Project serialization, media import, real preview, synchronized audio/video playback, effects processing, final rendering/export, undo/redo, and platform packaging still need implementation.

## Build the C++ application

Install a Qt 6.5+ kit with the Widgets module and a C++17-capable compiler, then configure from the repository root:

```sh
cmake -S . -B build
cmake --build build --config Release
```

Use the Qt kit and CMake toolchain appropriate to the target platform. Android and iOS builds require their respective Qt platform kits, SDKs and signing/deployment setup; they are not produced by a desktop build.

## Android transition project

`native-android/` is the existing Android prototype retained during migration. Its Java activity and Android-specific media classes are transitional and have **not** yet been replaced by the C++/Qt application. The Android CMake target now also compiles the shared C++ project model. The existing prototype preview and timeline are not yet feature parity with the cross-platform app.

## Legacy prototype

The old Flutter/Dart prototype remains in `.oldproto/` as a temporary reference. It is not part of the new C++ application. Do not remove it until its useful features have been ported and verified.

## Current status

The repository now has the first shared C++17 project model and a C++ Qt UI shell. No cross-platform release builds have been verified yet. Do not treat the five platform targets as completed until each has been built, run and tested on its own toolchain.
