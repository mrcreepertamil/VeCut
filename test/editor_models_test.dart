import 'package:flutter_test/flutter_test.dart';
import 'package:vecut/models/media_asset.dart';
import 'package:vecut/models/overlay_item.dart';
import 'package:vecut/models/timeline_clip.dart';
import 'package:vecut/models/timeline_track.dart';

void main() {
  group('TimelineClip', () {
    test('calculates source duration and copies with updated values', () {
      const clip = TimelineClip(
        id: 'clip-1',
        filePath: '/media/intro.mp4',
        sourceInMs: 1000,
        sourceOutMs: 6000,
        trackId: 'video-track',
      );

      expect(clip.durationMs, 5000);
      expect(clip.copyWith(startMs: 2500).trackId, 'video-track');
      expect(clip.copyWith(startMs: 2500).startMs, 2500);
    });
  });

  group('TimelineTrack', () {
    test('preserves identity and type when copied', () {
      const track = TimelineTrack(
        id: 'audio-1',
        name: 'Music',
        type: TimelineTrackType.audio,
      );

      final muted = track.copyWith(isMuted: true);
      expect(muted.id, 'audio-1');
      expect(muted.type, TimelineTrackType.audio);
      expect(muted.isMuted, isTrue);
    });
  });

  test('MediaAsset uses display name when provided', () {
    const asset = MediaAsset(
      id: 'asset-1',
      path: '/media/clip.mp4',
      type: MediaAssetType.video,
      displayName: 'Clip.mp4',
    );

    expect(asset.name, 'Clip.mp4');
  });

  test('OverlayItem stores normalized preview geometry', () {
    const overlay = OverlayItem(
      id: 'text-1',
      type: OverlayItemType.text,
      text: 'Hello',
      x: 0.5,
      y: 0.25,
    );

    expect(overlay.text, 'Hello');
    expect(overlay.x, 0.5);
    expect(overlay.y, 0.25);
  });
}
