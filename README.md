# VeCut

VeCut is a Flutter-based cross-platform video editor project, targeting Android first and Windows later.

## Current status

The app currently includes:
- A dark-themed home screen
- New-project setup with resolution and frame-rate choices
- A basic editor workspace shell

**This is an early UI foundation, not yet a functioning video editor.** Media importing, video playback, timeline editing, project saving, and exporting still need to be implemented.

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
