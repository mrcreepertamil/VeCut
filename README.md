# VeCut

**VeCut is an in-development, cross-platform video editor built with Flutter.** The project aims to provide a simple editing workspace for creating and organizing video projects, with Android as the first target and Windows planned as a later platform.

## Project goals

- Create video projects with configurable resolution and frame rate.
- Import and organize video and audio media.
- Preview clips and arrange them on a multi-track timeline.
- Add editing tools such as trimming, splitting, and transitions.
- Export finished videos.

## Current status

VeCut is at an early development stage. The repository currently contains the Flutter app foundation, a dark-themed home screen, project setup, and a basic editor workspace layout.

**The editing features listed above are goals, not completed features.** Media importing, real video playback, timeline editing, project saving, and video exporting still need to be implemented.

## Plugin development

VeCut is planned to support community plugins written in JavaScript. The initial plugin manifest, API draft, and example plugin are in the [plugins folder](plugins/).

**The plugin runtime and plugin manager are not implemented yet.** The current JavaScript files define the proposed developer format; they cannot run inside VeCut yet.

## Technology

- **Flutter / Dart** for the application interface.
- **Android** as the initial target platform.
- **Windows** as a planned desktop target.
- A dedicated media-processing engine is planned for later development.

## Run locally

Install the Flutter SDK, then run:

```bash
cd app
flutter create --platforms=android,windows .
flutter pub get
flutter run
```

## Build

Android APK:

```bash
cd app
flutter build apk --release
```

Windows desktop build (on Windows with Visual Studio C++ desktop tools installed):

```bash
cd app
flutter build windows --release
```

GitHub Actions workflows are included for Android and Windows builds.
