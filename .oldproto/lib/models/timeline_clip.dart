/// A video clip placed on a VeCut timeline.
class TimelineClip {
  const TimelineClip({
    required this.id,
    required this.filePath,
    this.startMs = 0,
    this.trackId,
    this.sourceInMs = 0,
    this.sourceOutMs,
  });

  final String id;
  final String filePath;
  final int startMs;
  final String? trackId;
  final int sourceInMs;
  final int? sourceOutMs;

  int? get durationMs =>
      sourceOutMs == null ? null : sourceOutMs! - sourceInMs;

  TimelineClip copyWith({
    String? id,
    String? filePath,
    int? startMs,
    String? trackId,
    int? sourceInMs,
    int? sourceOutMs,
  }) {
    return TimelineClip(
      id: id ?? this.id,
      filePath: filePath ?? this.filePath,
      startMs: startMs ?? this.startMs,
      trackId: trackId ?? this.trackId,
      sourceInMs: sourceInMs ?? this.sourceInMs,
      sourceOutMs: sourceOutMs ?? this.sourceOutMs,
    );
  }
}
