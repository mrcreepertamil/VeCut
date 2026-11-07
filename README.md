# VeCut

**VeCut is an in-development, cross-platform video editor built with Flutter.** Android is the first target; Windows is planned.

## Project goals

- Create projects with configurable resolution and frame rate.
- Import and organize video, audio, and image media.
- Preview clips and build a multi-track timeline.
- Add text and visual overlays.
- Trim, split, and arrange clips.
- Export finished videos.

## Current development status

The current editor workspace supports importing multiple video files, previewing a selected video, basic trimming and single-clip MP4 export. It also supports audio preview, text overlays, and image/video overlays in the preview.

The following are **not yet complete**:
- A continuous, synchronized multi-track timeline.
- Editing audio position, duration, and volume against the video.
- Overlay timing, transform controls, and compositing into exported video.
- Exporting the complete edited timeline rather than one trimmed source clip.
- Saving and reopening editable projects.

Supporting timeline, media-asset, and overlay data models plus a shared media picker service are being added as foundations. Their existence does not mean the full timeline or compositing engine is finished.

## Community plugins

VeCut is planned to support community plugins written in JavaScript. The plugin manifest, API draft, and example are in the [plugins folder](plugins/).

**The plugin runtime and plugin manager are not implemented yet.** The example files describe a proposed format and cannot currently run inside VeCut.

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
