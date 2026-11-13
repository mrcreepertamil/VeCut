# VeCut

**VeCut is an in-development video editor.** The project is migrating from Flutter/Dart toward a native Android application built with Java and C++, with JavaScript as the language for third-party community plugins.

The goal is to build a capable, performance-focused editor with its own identity and an extensible plugin ecosystem.

## Technology direction

- **Java** — native Android application, editor UI, project management, media library, timeline controls, and the JNI boundary.
- **C++17** — planned media engine for frame processing, rendering, effects, transitions, audio processing, and export.
- **OpenGL ES** — initial Android GPU renderer for preview, shaders, compositing, and effects. The interface currently creates a basic OpenGL preview surface; actual video-frame rendering is not implemented yet.
- **JavaScript** — third-party developer plugins through a deliberately limited plugin API.
- **Android** — first native target. Keep the C++ media engine as platform-independent as practical so future desktop ports remain possible.

Vulkan and a dedicated 4K optimization target are not part of the initial implementation plan.

## Native Android interface

The current native app includes an early editor-style interface:

- VeCut top bar and Export control.
- Preview panel backed by an OpenGL ES surface.
- Playback control placeholder and time display.
- Video and audio track placeholders.
- Timeline ruler and basic track selection.
- Tool strip for Media, Trim, Text, Effects, Audio, and Overlay.
- Android document picker for selecting video, image, and audio files.
- Basic engine and plugin status labels.

These controls establish the interface layout. Most editing actions are still placeholders: selecting a file does not yet decode it or add it to a functional timeline, and playback, trim, effects, text, audio mixing, overlays, and export are not implemented in the native app.

## Native migration status

The native foundation lives in [native-android](native-android/):

- Android Studio/Gradle project.
- Java editor interface and JNI bridge.
- C++ shared-library target built with CMake.
- OpenGL ES preview surface with ES 3.0 preference and ES 2.0 fallback.
- Embedded Rhino JavaScript runtime prototype.
- Bundled example plugin that registers an effect descriptor.

**This is a foundation, not feature parity.** The C++ media engine is currently a loading/bridge stub. The OpenGL renderer currently clears the preview surface; video decoding, frame upload, shaders, compositing, and GPU effects still need implementation. The JavaScript host demonstrates plugin logging and effect registration; public plugin installation, package management, robust isolation, and a stable plugin API are not ready.

The native app does not yet provide a complete editing workflow, project persistence, or final video export.

## Existing editor reference

The previous Flutter editor is retained in [app](app/) while its features are ported and verified in the native app. It remains a reference for the earlier UI and editing prototype. Keeping it temporarily avoids losing existing work during the migration.

## Build the native Android foundation

Open the [native-android](native-android/) folder in Android Studio with JDK 17, Android SDK 35, Android NDK, and CMake installed. Select the `app` run configuration and build/run on an Android device or emulator.

See [native-android/README.md](native-android/README.md) for details about the plugin API prototype, interface, and current limitations.

## Original Flutter prototype

The legacy project remains under `app/` and can still be used as a reference while native feature parity is developed.
