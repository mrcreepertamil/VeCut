# Android Java-to-C++ migration

VeCut's production application is the shared C++17/Qt application in
`apps/vecut/`. The old `native-android/` Java application is retained as a
reference until its remaining useful behavior has been ported and verified.
It is not the application architecture for iOS.

## Migration map

| Former Java class | C++ destination / replacement | Status |
| --- | --- | --- |
| `HomeActivity` | Qt Widgets application shell in `main.cpp` and `MainWindow` | The editor shell exists; a separate home screen is not yet ported. |
| `MainActivity` | `MainWindow` and `TimelineWidget` | Core editor UI and timeline operations are C++. |
| `AudioPreviewPlayer` | Qt Multimedia `QMediaPlayer` / `QAudioOutput` in `MainWindow` | Playback and multi-track audio preview exist in C++; lifecycle parity still needs testing. |
| `CodecVideoPlayer` | Qt Multimedia `QMediaPlayer` | Portable decoder path; platform hardware decoding is delegated to Qt/platform plugins. |
| `GlVideoPreview` | `QVideoSink`, `QImage`, and the C++ preview compositor in `MainWindow` | Selected-clip preview exists; full timeline compositing is still pending. |
| `MediaProbe` | `media_probe.h/.cpp` plus Qt Multimedia probing | Image dimensions and thumbnails are implemented in C++; video duration is probed by the existing Qt Multimedia path. |
| `NativeMediaEngine` | `engine/include/vecut/project.h` and `engine/src/project.cpp` | The shared C++ project/timeline model replaces the old JNI wrapper. |
| `ProjectStore` | Versioned `.vecut` project save/load in `MainWindow` | Replaced by portable project files rather than Android SharedPreferences. |
| `AppContextHolder` | No replacement required | Qt uses `QUrl` and Qt file APIs; no Android global ContentResolver is used. |
| `PluginHost` | Optional future plugin subsystem | Deliberately not ported into the editor core. JavaScript remains optional plugin scripting, not app/UI implementation. |

## Platform rule

Application, editor, timeline, media model, and preview orchestration stay in
C++17. Qt Multimedia supplies platform-specific media integration. Android's
MediaCodec and iOS's native media frameworks may be used internally by Qt's
platform plugins; VeCut does not add Java, Kotlin, Swift, or Objective-C
application/editor source to implement those features.

The legacy Java files are intentionally not deleted in this migration step.
The old Android Gradle prototype remains a reference until the equivalent
C++/Qt behavior is built and tested on Android and iOS.
