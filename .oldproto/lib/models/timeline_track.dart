import 'timeline_clip.dart';

enum TimelineTrackType { video, audio, text, overlay, effect }

/// A named lane in the editor timeline.
class TimelineTrack {
  const TimelineTrack({
    required this.id,
    required this.name,
    required this.type,
    this.clips = const [],
    this.isMuted = false,
    this.isVisible = true,
  });

  final String id;
  final String name;
  final TimelineTrackType type;
  final List<TimelineClip> clips;
  final bool isMuted;
  final bool isVisible;

  TimelineTrack copyWith({
    String? name,
    List<TimelineClip>? clips,
    bool? isMuted,
    bool? isVisible,
  }) {
    return TimelineTrack(
      id: id,
      name: name ?? this.name,
      type: type,
      clips: clips ?? this.clips,
      isMuted: isMuted ?? this.isMuted,
      isVisible: isVisible ?? this.isVisible,
    );
  }
}
