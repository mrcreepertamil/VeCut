# VeCut native Android foundation

This is the native Android application foundation:

- **Java** owns the Android editor interface, Android integration and JNI boundary.
- **C++17** now contains a thread-safe native timeline model. It can add media-source records, remove clips, store source trim ranges, clear the timeline and report its contents through JNI. It does not decode or render the media yet; codecs, playback, effects, mixing and export still need implementation.
- **OpenGL ES** backs the initial preview surface. The current renderer only clears the preview to a dark background; video-frame upload, shaders, compositing and GPU effects are not implemented yet. The app requests ES 3.0 when available and falls back to ES 2.0.
- **JavaScript** is the third-party plugin language. The initial Rhino-based host exposes only `veCut.log()` and `veCut.registerEffect()` and loads the bundled sample plugin. A real plugin manager, package installation, compatibility/version checks, and per-plugin lifecycle are still future work.

## Current interface

The native app now has an editor-style workspace with a preview panel, playback controls, video/audio tracks with imported clip cards, tool strip, media document picker and engine status panel. The media picker can select a document, inspect basic duration/dimensions with Android's `MediaMetadataRetriever`, and add its URI and MIME type to the native C++ timeline through JNI. Imported items appear as selectable clip cards on the video/image or audio track. Long-press a clip to remove it; select a clip and use Trim to save a source in/out range in milliseconds. These ranges are timeline metadata only: the preview still does not decode or play media, and edits are not yet saved into project files. Text, effects, overlays, audio mixing, playback and export remain unimplemented.

## Open and build

Open this `native-android` folder in Android Studio with JDK 17 and Android SDK 35 installed. Android Studio/Gradle will resolve the Android Gradle Plugin and Rhino dependency. Build and run the `app` configuration on an Android device or emulator with an NDK and CMake installed.

## Plugin API prototype

Place a plugin script in the app's assets during development and call `PluginHost.loadAssetPlugin()` with its asset path. A plugin can currently call:

```javascript
veCut.log("Hello from a plugin");
veCut.registerEffect({
  id: "my-effect",
  name: "My Effect",
  description: "An example effect"
});
```

This prototype deliberately does not expose Java objects, filesystem access, network access, or native pointers to plugin scripts. It is not yet a complete security boundary for untrusted third-party code; do not ship public plugin installation until isolation, execution limits, API validation, and permission prompts have been tested thoroughly.

## Migration status

The Flutter project in `../app` is retained temporarily as a reference while native feature parity is developed. It has not been deleted. The native app is not yet a complete replacement for the existing editor: media decoding, playback, a visual multi-track timeline, project persistence, rendering, effects, audio mixing and export have not been ported. Keep both until those features are implemented and verified, then remove the legacy project in a dedicated cleanup change.
