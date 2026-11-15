enum OverlayItemType { text, image, video }

/// A visual layer shown above the main video preview.
class OverlayItem {
  const OverlayItem({
    required this.id,
    required this.type,
    this.text,
    this.filePath,
    this.x = 0.5,
    this.y = 0.5,
    this.width = 0.3,
    this.height = 0.2,
    this.startMs = 0,
    this.endMs,
  });

  final String id;
  final OverlayItemType type;
  final String? text;
  final String? filePath;
  /// Normalized horizontal position (0.0 to 1.0).
  final double x;
  /// Normalized vertical position (0.0 to 1.0).
  final double y;
  /// Normalized width (0.0 to 1.0).
  final double width;
  /// Normalized height (0.0 to 1.0).
  final double height;
  final int startMs;
  final int? endMs;
}
