# VeCut native Android foundation

This is the native Android application foundation:

- **Java** owns the Android editor interface, Android integration and JNI boundary.
- **C++17** contains a thread-safe native timeline model exposed through JNI. It stores media-source records, durations, source trim ranges, ordering and split clips. It does not perform frame decoding or final rendering.
- **MediaCodec + OpenGL ES 2.0** provide video preview. `MediaExtractor` reads video tracks, `MediaCodec` decodes frames to a `Surface`, and a `SurfaceTexture` external OES texture is drawn by an OpenGL shader. Video clips can be previewed sequentially in imported order with fade-through-black transitions. The source-time seek bar restarts decoding from the nearest preceding sync frame, and Split can create a second native timeline clip at the selected source time. Clip-to-clip preview transitions fade through black. The preview shader offers normal, monochrome, warm and cool looks. **MediaPlayer** provides standalone audio-source preview. These are preview features, not yet a synchronized final timeline renderer.
- **JavaScript** is the third-party plugin language. The initial Rhino-based host exposes only `veCut.log()` and `veCut.registerEffect()` and loads the bundled sample plugin. A real plugin manager, package installation, compatibility/version checks, and per-plugin lifecycle are still future work.

## Current interface

The native app has an editor-style workspace with a preview panel, playback controls, video/audio tracks, a tool strip and a media document picker. Video imports can play in the preview: the app opens the document with `MediaExtractor`, configures a `MediaCodec` decoder to output to the preview `Surface`, and draws the resulting `SurfaceTexture` frames with an OpenGL ES external-texture shader. The preview supports play, pause, lifecycle pause/resume and replay after reaching the end. Selecting a video clip starts preview from that clip and continues through the imported video sources in order. The Effects tool applies normal, monochrome, warm or cool shader looks to the live video preview. Audio imports use Android `MediaPlayer` for standalone audio preview; audio is not yet mixed with video or synchronized to the timeline. Images continue to use an Android `ImageView` still preview. The media picker inspects basic duration/dimensions with `MediaMetadataRetriever` and adds the source and metadata to the native C++ timeline through JNI. Imported source URIs, MIME types and display names are stored locally and restored on launch. Long-press a clip to remove it; select a clip and use Trim to save a source in/out range in milliseconds, or use Split to split at the seek-bar position. Trim metadata is not persisted yet and is not yet applied to playlist playback. Seeking is source-time preview seeking, not frame-accurate timeline scrubbing.

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

The Flutter project in `../.oldproto` is retained temporarily as a reference while native feature parity is developed. It has not been deleted. The native app is not yet a complete replacement for the existing editor: timeline-synchronized multi-clip playback, audio decoding/mixing, seeking and frame-accurate scrubbing, effects, transitions, text, overlays, portable project-file import/export and final rendering/export remain to be implemented. Keep both until those features are implemented and verified, then remove the legacy project in a dedicated cleanup change.
