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

The current editor workspace supports:
- Importing multiple videos and images into a project Media Library.
- Browsing imported assets and adding them to the timeline.
- Previewing a selected video and displaying image/video/text overlays.
- Adding clips to selectable timeline tracks and removing placed clips.
- Basic trim controls and single-source MP4 export.
- Audio import, preview, a timeline start offset and volume controls, plus basic playback synchronization with the preview.
- Overlay position, scale, start-time and duration controls in the preview.

The following are **still in development**:
- A true frame-accurate, continuously rendered multi-track timeline.
- Keeping audio synchronized throughout long playback sessions and mixing multiple audio clips.
- Rendering image/video/text overlays and all timeline tracks into the final exported video.
- Exporting the complete edited timeline rather than one trimmed source clip.
- Saving and reopening editable project files.
- Undo/redo, split, and the other advanced timeline toolbar actions.

The current media library and timeline controls are functional editing foundations, but the project is not yet a complete professional multi-track editor.

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
