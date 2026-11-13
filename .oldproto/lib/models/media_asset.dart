enum MediaAssetType { video, audio, image }

/// A source file imported into a VeCut project.
class MediaAsset {
  const MediaAsset({
    required this.id,
    required this.path,
    required this.type,
    this.displayName,
    this.durationMs,
  });

  final String id;
  final String path;
  final MediaAssetType type;
  final String? displayName;
  final int? durationMs;

  String get name => displayName ?? path.split(RegExp(r'[/\\\\]')).last;
}
