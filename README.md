# VeCut

**VeCut is an in-development cross-platform video editor.** The target is one C++17 application and one shared editor experience on Windows, Android, Linux, macOS and iOS.

VeCut's editor UI and editing engine are implemented in C++17. Flutter and Dart application code and build dependencies have been removed from the repository. Operating-system APIs may still be used behind narrow integration layers for media codecs, files, windowing, packaging and device capabilities; those layers are not separate editor implementations.

## Technology direction

- **C++17** — application UI, project model, timeline, editing operations, effects, transitions, audio graph and render pipeline.
- **Qt 6 Widgets** — the initial all-C++ cross-platform UI framework. The same UI target is intended to be configured for Windows, Android, Linux, macOS and iOS using the relevant Qt kits/toolchains.
- **CMake** — shared build configuration and reusable engine target.
- **Platform media backends** — selected behind C++ interfaces as the engine grows. The existing Android MediaCodec/OpenGL ES preview is transitional and is not yet the cross-platform renderer.
- **JavaScript plugins** — optional future third-party plugin scripting only; not used to implement the editor or its UI.

## Responsive application workspace

The Qt UI switches layout based on available window width:

- **Phone portrait (narrower than 720 px):** media bin above the preview, with a compact timeline below. Track buttons use shorter labels and the window can shrink to phone-sized dimensions.
- **Desktop landscape (720 px and wider):** media bin beside the larger preview, with the timeline spanning the bottom.
- The same C++ widgets and project state are used in both layouts; this is a responsive rearrangement, not separate Android/iOS and PC editor implementations.

The mobile layout is designed for portrait use, but Android/iOS packaging, touch-device testing, and platform-specific file-picker integration still need to be built and verified with the appropriate Qt kits.

## C++ application and engine

- `engine/` contains the platform-independent C++17 project model.
- The model represents media assets, video/audio/overlay tracks, clips, source trim ranges, incoming transitions, clip volume, splitting, clip ordering and project validation.
- `apps/vecut/` contains the Qt Widgets application, written entirely in C++.
- The workspace has a multi-file media import dialog, a media bin, selected-media details, image thumbnails, file-size reporting, and controls to create video/audio tracks.
- Qt Multimedia asynchronously probes audio/video duration; Qt Image I/O reads still-image dimensions and thumbnails. Unknown durations remain unknown rather than being guessed.
- Media-bin items can be dragged onto compatible audio/video tracks at a chosen timeline position. Still images use a five-second default clip duration; video/audio drops wait for duration probing to finish.
- Projects can be saved and reopened as versioned `.vecut` JSON documents. The C++ Qt editor persists media references and metadata, track settings, clip timing and trims, transitions, and volume; project loading validates the structure before replacing the current project.
- The timeline renders a time ruler and visual clip blocks on separate track lanes. Drag a clip body horizontally to reposition it; drag either edge to trim the source range. Select a clip to preview its source, then use the native Split, Trim, Transition, Volume, and Remove clip controls. Per-clip gain (0×–4×) is validated by the C++ project model and saved in `.vecut` projects. Split can use the preview playhead as its initial split point, and transition choices are stored in the shared C++ project model.
- The Transition editor stores incoming Fade, Dissolve, Wipe and Slide transitions, plus fade-in and fade-out durations as part of each clip's transition settings. Main video and overlay clips have separate per-clip Zoom, Pan and Rotate animation settings with a duration. These settings are validated and saved in `.vecut` projects. The selected-clip preview now applies transition fade timing, Fade/Dissolve opacity, Wipe and Slide reveals, and Zoom/Pan/Rotate transforms to decoded video frames and still images through a shared Qt video-frame/image compositor. This is a live selected-clip effects preview, not yet a multi-clip timeline compositor or final export renderer.
- The **Play timeline audio** control schedules audio-bearing clips on every unmuted track, including audio files on audio tracks and embedded audio from video clips at its timeline offset and mixes overlapping clips through independent Qt Multimedia players. Each channel uses its clip's saved volume (playback currently caps gain at 1× because Qt's standard output volume control does not provide amplification). This is an initial audio-track mixer; synchronized multi-clip video playback, waveform display, bus routing and final export rendering remain unfinished.
- Clip dragging between different tracks, undo/redo and export are still unfinished.
- Engine unit tests cover clip ordering (including moving to the end), trimming, transitions, per-clip volume validation, splitting, validation and removal. Audio playback mixing is currently an application-level Qt Multimedia feature and does not yet have offline-render tests.

## Build

To build the desktop application, install a Qt 6.5+ kit with the Widgets and Multimedia modules and a C++17-capable compiler:

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

## Current status

The shared C++17 project model, first engine tests, responsive Qt media-bin/track workspace, and initial native C++ project save/load are in place. The repository's Windows and Android CI workflows target the C++/Qt application; their successful execution and the cross-platform release builds still need to be verified. Do not treat the five platform targets as completed until each has been built, run and tested on its own toolchain.
