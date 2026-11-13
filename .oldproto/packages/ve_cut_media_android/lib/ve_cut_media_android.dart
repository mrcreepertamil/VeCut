import 'package:flutter/services.dart';

/// Metadata read from a video file by Android's media framework.
class VeCutVideoInfo {
  const VeCutVideoInfo({
    required this.durationMs,
    required this.width,
    required this.height,
    required this.rotation,
  });

  final int durationMs;
  final int width;
  final int height;
  final int rotation;

  Duration get duration => Duration(milliseconds: durationMs);

  factory VeCutVideoInfo.fromMap(Map<Object?, Object?> map) {
    return VeCutVideoInfo(
      durationMs: (map['durationMs'] as num?)?.toInt() ?? 0,
      width: (map['width'] as num?)?.toInt() ?? 0,
      height: (map['height'] as num?)?.toInt() ?? 0,
      rotation: (map['rotation'] as num?)?.toInt() ?? 0,
    );
  }
}

/// Calls the VeCut Android plugin through a Flutter platform channel.
class VeCutMediaAndroid {
  VeCutMediaAndroid._();

  static const MethodChannel _channel = MethodChannel('ve_cut_media_android');

  /// Reads duration, dimensions, and rotation from a local video path.
  ///
  /// This method is currently supported on Android only.
  static Future<VeCutVideoInfo> getVideoInfo(String path) async {
    final result = await _channel.invokeMapMethod<Object?, Object?>(
      'getVideoInfo',
      <String, Object?>{'path': path},
    );

    if (result == null) {
      throw PlatformException(
        code: 'EMPTY_MEDIA_INFO',
        message: 'Android returned no video metadata.',
      );
    }

    return VeCutVideoInfo.fromMap(result);
  }
}
