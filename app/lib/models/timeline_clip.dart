/// A video clip placed on a VeCut timeline.
class TimelineClip {
  const TimelineClip({
    required this.id,
    required this.filePath,
    this.startMs = 0,
    this.sourceInMs = 0,
    this.sourceOutMs,
  });

  final String id;
  final String filePath;
  final int startMs;
  final int sourceInMs;
  final int? sourceOutMs;

  int? get durationMs =>
      sourceOutMs == null ? null : sourceOutMs! - sourceInMs;

  TimelineClip copyWith({
    String? id,
    String? filePath,
    int? startMs,
    int? sourceInMs,
    int? sourceOutMs,
  }) {
    return TimelineClip(
      id: id ?? this.id,
      filePath: filePath ?? this.filePath,
      startMs: startMs ?? this.startMs,
      sourceInMs: sourceInMs ?? this.sourceInMs,
      sourceOutMs: sourceOutMs ?? this.sourceOutMs,
    );
  }
}
