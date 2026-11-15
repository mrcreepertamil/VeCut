# VeCut

**VeCut is an in-development video editor.** The project is migrating from Flutter/Dart toward a native Android application built with Java and C++, with JavaScript as the language for third-party community plugins.

The goal is to build a capable, performance-focused editor with its own identity and an extensible plugin ecosystem.

## Technology direction

- **Java** — native Android application, editor UI, project management, media library, timeline controls, and the JNI boundary.
- **C++17** — native timeline model and JNI bridge. Android's Java media layer currently handles video decoding; native frame processing, effects, transitions, audio processing, and export remain future work.
- **OpenGL ES 2.0** — displays decoded video frames through an external `SurfaceTexture` using a GPU shader. Android `MediaCodec` decodes the selected video directly to the preview surface.
- **JavaScript** — third-party developer plugins through a deliberately limited plugin API.
- **Android** — first native target. Keep the C++ media engine as platform-independent as practical so future desktop ports remain possible.

Vulkan and a dedicated 4K optimization target are not part of the initial implementation plan.

## Native Android interface

The current native app includes an early editor-style interface:

- VeCut top bar and Export control.
- Preview panel backed by an OpenGL ES external texture and Android `MediaCodec` decoder.
- Basic video play/pause/replay controls; timeline timecode and seeking are not connected yet.
- Video and audio track placeholders.
- Timeline ruler and basic track selection.
- Tool strip for Media, Trim, Text, Effects, Audio, and Overlay.
- Android document picker for selecting video, image, and audio files.
- Basic engine and plugin status labels.

Video imports can now be decoded with Android `MediaCodec` and displayed through an OpenGL ES `SurfaceTexture` preview. The native C++ timeline stores clip sources, durations, and source trim ranges. The playback path is an early preview implementation: audio playback/mixing, timeline-synchronized multi-clip playback, seeking, frame-accurate editing, effects, transitions, text, overlays, and final export are not implemented yet.

## Native migration status

The native foundation lives in [native-android](native-android/):

- Android Studio/Gradle project.
- Java editor interface and JNI bridge.
- C++ shared-library target built with CMake.
- OpenGL ES 2.0 external-texture preview surface fed by an Android `MediaCodec` video decoder.
- Embedded Rhino JavaScript runtime prototype.
- Bundled example plugin that registers an effect descriptor.

**This is a foundation, not feature parity.** The Android video preview now decodes one selected video stream with `MediaCodec` and renders frames through an OpenGL ES external texture. It does not yet mix audio, sequence multiple timeline clips, support seeking or frame-accurate scrubbing, apply effects, composite overlays, or export a finished video. The C++ layer remains a timeline model and JNI bridge rather than a full render engine. The JavaScript host demonstrates plugin logging and effect registration; public plugin installation, package management, robust isolation, and a stable plugin API are not ready.

The native app does not yet provide a complete editing workflow, project persistence, or final video export.

## Archived Flutter prototype

The previous Flutter editor is retained in [.oldproto](.oldproto/) while its features are ported and verified in the native app. It remains a reference for the earlier UI and editing prototype. Keeping it temporarily avoids losing existing work during the migration.

## Build the native Android foundation

Open the [native-android](native-android/) folder in Android Studio with JDK 17, Android SDK 35, Android NDK, and CMake installed. Select the `app` run configuration and build/run on an Android device or emulator.

See [native-android/README.md](native-android/README.md) for details about the plugin API prototype, interface, and current limitations.

## Original Flutter prototype (`.oldproto/`)

The legacy project remains under `.oldproto/` and can still be used as a reference while native feature parity is developed.
