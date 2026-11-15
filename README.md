# VeCut

**VeCut is an in-development cross-platform video editor.** The long-term target is one C++17 application and one shared editor experience on Windows, Android, Linux, macOS and iOS.

The project is moving away from Flutter/Dart and separate platform-language editor implementations. The application UI and editing engine are being built in C++17. Operating-system APIs may still be used behind narrow integration layers for media codecs, files, windowing, packaging and device capabilities; those layers are not separate editor implementations.

## Technology direction

- **C++17** — application UI, project model, timeline, editing operations, effects, transitions, audio graph and render pipeline.
- **Qt 6 Widgets** — the initial all-C++ cross-platform UI framework. The same UI target is intended to be configured for Windows, Android, Linux, macOS and iOS using the relevant Qt kits/toolchains.
- **CMake** — shared build configuration and reusable engine target.
- **Platform media backends** — selected behind C++ interfaces as the engine grows. The existing Android MediaCodec/OpenGL ES preview is transitional and is not yet the cross-platform renderer.
- **JavaScript plugins** — optional future third-party plugin scripting only; not used to implement the editor or its UI.

## C++ application and engine

- `engine/` contains the platform-independent C++17 project model.
- The model represents media assets, video/audio/overlay tracks, clips, source trim ranges, incoming transitions, clip volume, splitting, clip ordering and project validation.
- `apps/vecut/` contains the Qt Widgets application, written entirely in C++.
- The desktop workspace now has a media-bin import dialog, selected-media information, preview placeholder, and controls to create video/audio tracks.
- Media files are listed in the project, but duration probing, placing clips on tracks, decoding, playback and export are not connected yet. The UI deliberately does not invent media durations.
- Engine unit tests cover clip ordering (including moving to the end), trimming, transitions, splitting, validation and removal.

## Build

To build the desktop application, install a Qt 6.5+ kit with the Widgets module and a C++17-capable compiler:

```sh
cmake -S . -B build
cmake --build build --config Release
```

To configure and run only the platform-independent engine tests without Qt:

```sh
cmake -S . -B build-engine -DVECUT_BUILD_DESKTOP=OFF -DBUILD_TESTING=ON
cmake --build build-engine
ctest --test-dir build-engine --output-on-failure
```

Use the Qt kit and CMake toolchain appropriate to the target platform. Android and iOS builds require their respective Qt platform kits, SDKs and signing/deployment setup; they are not produced by a desktop build.

## Android transition project

`native-android/` is the existing Android prototype retained during migration. Its Java activity and Android-specific media classes are transitional and have **not** yet been replaced by the C++/Qt application. The Android CMake target also compiles the shared C++ project model. The existing prototype preview and timeline are not yet feature parity with the cross-platform app.

## Legacy prototype

The old Flutter/Dart prototype remains in `.oldproto/` as a temporary reference. It is not part of the new C++ application. Do not remove it until its useful features have been ported and verified.

## Current status

The shared C++17 project model, first engine tests, and initial Qt media-bin/track workspace are in place. No local or CI build has been verified for this update, and no cross-platform release builds have been verified. Do not treat the five platform targets as completed until each has been built, run and tested on its own toolchain.
