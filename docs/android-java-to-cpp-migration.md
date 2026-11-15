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
| `AudioPreviewPlayer` | `apps/vecut/audio_preview.h/.cpp` (`AudioPreview`) | Dedicated C++/Qt audio-only preview class is integrated into the editor; lifecycle parity still needs testing. |
| `CodecVideoPlayer` | `apps/vecut/video_preview.h/.cpp` (`VideoPreview`) | Dedicated C++/Qt video preview class is integrated; platform decoder and hardware acceleration are supplied by Qt platform plugins, not a copied Android MediaCodec implementation. |
| `GlVideoPreview` | `VideoPreview`, `preview_compositor.h/.cpp` and `gpu_preview_widget.h/.cpp` | C++/Qt playback, selected-clip composition and first GPU frame/filter preview are implemented; full timeline compositing remains pending. |
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


## Dedicated playback classes

The editor now owns separate reusable `VideoPreview` and `AudioPreview`
classes. Both are implemented in C++ and use Qt Multimedia. The editor routes
video and audio media to the matching player, while image clips continue to
use the still-image preview path. This is a portable abstraction rather than
a line-for-line port of Android's Java `MediaCodec` or OpenGL ES code.

The Qt platform plugin chooses the platform decoder and audio output. Actual
Android/iOS device playback, lifecycle behavior, codec compatibility, and
hardware acceleration still require builds and device tests on those platforms.

## Preview compositor extraction

The selected-clip frame composition logic now lives in
`apps/vecut/preview_compositor.h/.cpp`, separate from the main window. It
accepts a decoded/still `QImage`, target size, selected clip and elapsed clip
time, then returns a composed frame. The clip transforms and transition composition still use QPainter on the CPU.
The legacy Android shader looks are available through a portable GLSL shader
library and are executed by the GPU preview widget for color filtering. This is
not an adjacent-clip compositor or export renderer.


## Cross-platform GPU preview renderer

The Qt preview stack now includes `GpuPreviewWidget`, a C++ `QOpenGLWidget`
renderer using `QOpenGLFunctions` and `QOpenGLShaderProgram`. Decoded video
frames and still-image frames are first processed by the reusable clip
compositor, then uploaded as RGBA textures and drawn through a GLSL filter
shader. The preview filter selector exposes Normal, Monochrome, Warm, Cool,
Sepia, Vintage, Invert and Contrast.

The shader library supplies OpenGL ES 2.0 syntax for GLES contexts (the mobile
Android/iOS path) and GLSL 1.20 syntax for desktop compatibility contexts.
The widget falls back to drawing the frame with QPainter if shader compilation
or linking fails. The app links Qt 6 OpenGLWidgets; all VeCut renderer and UI
code remains C++.

This is a first GPU frame/filter renderer, not a GPU-native video decoder,
multi-clip timeline compositor, transition compositor, or export pipeline.
Frames are still delivered as QImages by Qt Multimedia and uploaded to a GL
texture. Actual Android/iOS device builds, context creation, shader compilation,
orientation, lifecycle and performance must be verified on hardware before
claiming platform support.
