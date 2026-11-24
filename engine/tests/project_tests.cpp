#include "vecut/project.h"

#include <algorithm>
#include <cassert>
#include <string>

using namespace vecut;

int main() {
    Project project("Engine tests");

    MediaAsset media;
    media.id = "media-1";
    media.uri = "file:///sample.mp4";
    media.display_name = "sample.mp4";
    media.kind = MediaKind::Video;
    media.duration_ms = 10000;
    assert(project.add_media(media));
    assert(project.update_media_info("media-1", 10000, 1920, 1080));
    assert(project.media().front().width == 1920);
    assert(project.media().front().height == 1080);
    assert(!project.update_media_info("missing-media", 1000, 10, 10));
    assert(!project.update_media_info("media-1", -1, 10, 10));
    MediaAsset invalid_dimensions = media;
    invalid_dimensions.id = "invalid-dimensions";
    invalid_dimensions.width = -1;
    assert(!project.add_media(invalid_dimensions));

    Track track;
    track.id = "video-1";
    track.name = "Video 1";
    track.kind = TrackKind::Video;
    assert(project.add_track(track));

    Clip first;
    first.id = "clip-1";
    first.media_id = media.id;
    first.source_in_ms = 0;
    first.source_out_ms = 4000;
    assert(project.add_clip("video-1", first));
    Clip invalid_volume = first;
    invalid_volume.id = "invalid-volume";
    invalid_volume.volume = 5.0;
    assert(!project.add_clip("video-1", invalid_volume));
    assert(project.tracks().front().clips.back().timeline_start_ms == 0);

    Clip second = first;
    second.id = "clip-2";
    second.source_in_ms = 4000;
    second.source_out_ms = 8000;
    assert(project.add_clip("video-1", second));
    assert(project.tracks().front().clips.back().timeline_start_ms == 4000);

    Clip third = first;
    third.id = "clip-3";
    third.source_in_ms = 8000;
    third.source_out_ms = 10000;
    assert(project.add_clip("video-1", third));
    assert(project.tracks().front().clips.back().timeline_start_ms == 8000);

    // Moving a clip to the final index must remain within vector bounds.
    assert(project.move_clip("video-1", "clip-1", 2));
    assert(project.tracks().front().clips.back().id == "clip-1");

    // Moving the last clip to the first index and a no-op move both work.
    assert(project.move_clip("video-1", "clip-1", 0));
    assert(project.tracks().front().clips.front().id == "clip-1");
    assert(project.move_clip("video-1", "clip-1", 0));
    assert(!project.move_clip("video-1", "missing", 0));

    assert(project.trim_clip("video-1", "clip-1", 500, 3500));
    assert(!project.trim_clip("video-1", "clip-1", -1, 3500));

    assert(project.set_clip_volume("video-1", "clip-1", 0.65));
    assert(project.tracks().front().clips.front().volume == 0.65);
    assert(project.set_clip_volume("video-1", "clip-1", 0.0));
    assert(!project.set_clip_volume("video-1", "clip-1", 4.1));
    assert(!project.set_clip_volume("video-1", "missing", 1.0));

    assert(project.set_transition("video-1", "clip-1",
                                  Transition{TransitionKind::Fade, 500}));
    assert(!project.set_transition("video-1", "clip-1",
                                   Transition{TransitionKind::Fade, 5000}));
    assert(project.set_clip_animation("video-1", "clip-1", ClipAnimation{AnimationKind::ZoomIn, 800}));
    assert(!project.set_clip_animation("video-1", "clip-1", ClipAnimation{AnimationKind::ZoomOut, 9000}));
    auto transition_with_fades = project.tracks().front().clips.front().incoming_transition;
    transition_with_fades.fade_in_ms = 300;
    transition_with_fades.fade_out_ms = 450;
    assert(project.set_transition("video-1", "clip-1", transition_with_fades));
    assert(project.tracks().front().clips.front().incoming_transition.fade_in_ms == 300);
    assert(project.tracks().front().clips.front().incoming_transition.fade_out_ms == 450);
    transition_with_fades.fade_in_ms = 5000;
    assert(!project.set_transition("video-1", "clip-1", transition_with_fades));

    const auto right_id = project.split_clip("video-1", "clip-1", 2000);
    assert(right_id.has_value());
    assert(project.tracks().front().clips.size() == 4);
    const auto& split_clips = project.tracks().front().clips;
    const auto right_clip = std::find_if(split_clips.begin(), split_clips.end(),
        [&](const Clip& clip) { return clip.id == *right_id; });
    assert(right_clip != split_clips.end());
    assert(right_clip->timeline_start_ms == 1500);
    assert(project.move_clip_to_time("video-1", "clip-1", 12000));
    assert(!project.move_clip_to_time("video-1", "missing", 0));
    assert(!project.move_clip_to_time("video-1", "clip-1", -1));

    Track second_video_track;
    second_video_track.id = "video-2";
    second_video_track.kind = TrackKind::Video;
    assert(project.add_track(second_video_track));
    assert(project.move_clip_to_track("video-1", "clip-2", "video-2"));
    assert(project.tracks()[1].clips.size() == 1);
    assert(project.tracks()[1].clips.front().id == "clip-2");
    assert(!project.move_clip_to_track("video-1", "clip-2", "video-2"));
    assert(!project.move_clip_to_track("video-1", "clip-1", "missing-track"));

    std::string error;
    assert(project.validate(&error));
    assert(error.empty());
    assert(project.duration_ms() >= 12000);

    assert(project.remove_clip("video-1", *right_id));
    assert(!project.remove_clip("video-1", *right_id));

    Project still_project("Still image clip test");
    MediaAsset still;
    still.id = "still-1";
    still.uri = "file:///still.png";
    still.display_name = "still.png";
    still.kind = MediaKind::Image;
    still.duration_ms = 5000;
    assert(still_project.add_media(still));

    Track still_track;
    still_track.id = "still-video";
    still_track.kind = TrackKind::Video;
    assert(still_project.add_track(still_track));
    Track overlay_track;
    overlay_track.id = "overlay-1";
    overlay_track.kind = TrackKind::Overlay;
    assert(still_project.add_track(overlay_track));

    Clip still_clip;
    still_clip.id = "still-clip";
    still_clip.media_id = still.id;
    still_clip.source_in_ms = 0;
    still_clip.source_out_ms = 5000;
    assert(still_project.add_clip("still-video", still_clip));
    assert(still_project.set_clip_animation("still-video", "still-clip", ClipAnimation{AnimationKind::PanLeft, 1000}));
    auto still_transition = still_project.tracks().front().clips.front().incoming_transition;
    still_transition.fade_in_ms = 250;
    still_transition.fade_out_ms = 250;
    assert(still_project.set_transition("still-video", "still-clip", still_transition));
    Clip overlay_clip = still_clip;
    overlay_clip.id = "overlay-clip";
    assert(still_project.add_clip("overlay-1", overlay_clip));
    assert(still_project.set_clip_animation("overlay-1", "overlay-clip", ClipAnimation{AnimationKind::RotateClockwise, 700}));

    // Track/media compatibility is enforced by the engine, not only the UI.
    Track audio_track;
    audio_track.id = "audio-only";
    audio_track.kind = TrackKind::Audio;
    assert(still_project.add_track(audio_track));
    Clip invalid_audio_clip = still_clip;
    invalid_audio_clip.id = "invalid-audio-track-clip";
    assert(!still_project.add_clip("audio-only", invalid_audio_clip));

    MediaAsset audio;
    audio.id = "audio-1";
    audio.uri = "file:///audio.wav";
    audio.display_name = "audio.wav";
    audio.kind = MediaKind::Audio;
    audio.duration_ms = 3000;
    assert(still_project.add_media(audio));
    assert(!still_project.move_clip_to_track("still-video", "still-clip", "audio-only"));
    Track second_audio_track;
    second_audio_track.id = "audio-only-2";
    second_audio_track.kind = TrackKind::Audio;
    assert(still_project.add_track(second_audio_track));
    Clip audio_clip;
    audio_clip.id = "audio-clip";
    audio_clip.media_id = audio.id;
    audio_clip.source_out_ms = 2000;
    assert(still_project.add_clip("audio-only", audio_clip));
    assert(still_project.move_clip_to_track("audio-only", "audio-clip", "audio-only-2"));
    assert(still_project.tracks().back().clips.front().id == "audio-clip");
    assert(!still_project.move_clip_to_track("audio-only", "missing-clip", "audio-only-2"));
    Clip invalid_video_clip;
    invalid_video_clip.id = "invalid-video-track-clip";
    invalid_video_clip.media_id = audio.id;
    invalid_video_clip.source_out_ms = 1000;
    assert(!still_project.add_clip("still-video", invalid_video_clip));

    return 0;
}
