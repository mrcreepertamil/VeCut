# VeCut

**VeCut is an in-development video editor.** The project is beginning a migration from Flutter/Dart to a native Android application built with Java and C++, with JavaScript as the language for third-party community plugins.

## Technology direction

- **Java** — native Android application, UI, project management, media library, timeline controls, and the JNI boundary.
- **C++17** — planned media engine for rendering, effects, transitions, audio processing, and export.
- **JavaScript** — third-party developer plugins through a deliberately limited plugin API.
- **Android** — first native target. A shared C++ engine may support a future Windows application.

## Native migration status

The initial native foundation lives in [native-android](native-android/):

- Native Android Studio/Gradle project.
- Java launcher screen and JNI bridge.
- C++ shared-library target built with CMake.
- Embedded Rhino JavaScript runtime prototype.
- A bundled example plugin that registers an effect descriptor.

**This is a foundation, not feature parity yet.** The C++ media engine is currently a loading stub. The JavaScript host currently demonstrates plugin logging and effect registration; public plugin installation, package management, full isolation, and a stable plugin API are not ready. The native app does not yet contain the previous editor's media import, timeline, preview, project persistence, or export functionality.

## Existing editor reference

The previous Flutter editor is retained in [app](app/) while its features are ported and verified in the native app. It currently provides the earlier UI and editing prototype, including media import, basic timeline controls, preview, audio controls, and limited single-source export. Keeping it temporarily avoids losing working reference code during the migration.

## Build the native Android foundation

Open the [native-android](native-android/) folder in Android Studio with JDK 17, Android SDK 35, Android NDK, and CMake installed. Select the `app` run configuration and build/run on an Android device or emulator.

See [native-android/README.md](native-android/README.md) for the current plugin API prototype and migration limitations.

## Original Flutter prototype

The legacy project remains under `app/` and can still be run with Flutter while native feature parity is developed.
