#include "vecut/project.h"

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

    Clip second = first;
    second.id = "clip-2";
    second.source_in_ms = 4000;
    second.source_out_ms = 8000;
    assert(project.add_clip("video-1", second));

    Clip third = first;
    third.id = "clip-3";
    third.source_in_ms = 8000;
    third.source_out_ms = 10000;
    assert(project.add_clip("video-1", third));

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

    assert(project.set_transition("video-1", "clip-1",
                                  Transition{TransitionKind::Fade, 500}));
    assert(!project.set_transition("video-1", "clip-1",
                                   Transition{TransitionKind::Fade, 5000}));

    const auto right_id = project.split_clip("video-1", "clip-1", 2000);
    assert(right_id.has_value());
    assert(project.tracks().front().clips.size() == 4);

    std::string error;
    assert(project.validate(&error));
    assert(error.empty());
    assert(project.duration_ms() > 0);

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

    Clip still_clip;
    still_clip.id = "still-clip";
    still_clip.media_id = still.id;
    still_clip.source_in_ms = 0;
    still_clip.source_out_ms = 5000;
    assert(still_project.add_clip("still-video", still_clip));
    return 0;
}
