# Android C++ implementation

VeCut's Android editor is the same C++17/Qt application used on desktop:
`apps/vecut/`. The old standalone Java/Gradle prototype has been retired;
its editor UI and media-preview responsibilities are superseded by the
shared Qt Widgets, Qt Multimedia and C++ preview implementation.

## Language boundary

- Editor UI, project model, timeline, media scanning orchestration and preview
  code are C++17.
- Android-specific MediaStore access is implemented in
  `apps/vecut/media_scanner_android.cpp`, using JNI from C++ to call Android's
  platform APIs. There is no app-owned Java source file.
- Qt supplies the Android activity/bootstrap and platform integration.
- The Android SDK packaging toolchain still requires a JDK. This is a build
  dependency, not a Java editor implementation.
- JavaScript remains optional plugin scripting only; it is not used for the
  application or editor UI.

## Android media scanning

The C++ scanner queries the external Images, Video and Audio MediaStore
collections through Android's ContentResolver. It collects available file
paths, tolerates unavailable permissions/provider columns, and returns the
paths to the existing C++ media-bin pipeline. Runtime media permissions are
requested by the C++ Qt application through JNI.

The scanner retains the existing path-based media-bin behavior. Android
scoped-storage access can vary by device and Android version; content-URI
based import and playback remains a future improvement.

## Build and verification

The Android APK workflow builds the C++ Qt target `VeCut` for arm64 using
the Android Qt kit, SDK and NDK. It also installs JDK 17 because Qt's Android
deployment and packaging tools require Java tooling. A successful APK build
does not replace on-device testing of media permissions, MediaStore access,
decoder compatibility, lifecycle behavior, OpenGL context creation or
performance.

The former `native-android/` Gradle prototype and its Java activities,
players, JNI wrapper and project store have been removed from the active
repository. Their source remains available in earlier Git history if a
specific behavior needs to be recovered.
