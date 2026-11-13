# VeCut native Android foundation

This is the new native Android application foundation:

- **Java** owns the Android application and JNI boundary.
- **C++17** is the planned media engine. The current native library is only a loading/bridge stub; timeline rendering, codecs, effects, mixing and export still need implementation.
- **JavaScript** is the third-party plugin language. The initial Rhino-based host exposes only `veCut.log()` and `veCut.registerEffect()` and loads the bundled sample plugin. A real plugin manager, package installation, compatibility/version checks, and per-plugin lifecycle are still future work.

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

The Flutter project in `../app` is retained temporarily as a reference while native feature parity is developed. It has not been deleted. The native foundation is not yet a replacement for the existing editor: media import, timeline editing, preview, project persistence, and export have not been ported. Keep both until those features are implemented and verified, then remove the legacy project in a dedicated cleanup change.
