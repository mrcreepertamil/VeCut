#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace vecut {

enum class MediaKind { Video, Audio, Image };
enum class TrackKind { Video, Audio, Overlay };
enum class TransitionKind { None, Fade, Dissolve, WipeLeftToRight, WipeRightToLeft, SlideLeftToRight, SlideRightToLeft };
enum class AnimationKind { None, ZoomIn, ZoomOut, PanLeft, PanRight, PanUp, PanDown, RotateClockwise, RotateCounterClockwise };

struct MediaAsset {
    std::string id;
    std::string uri;
    std::string display_name;
    MediaKind kind = MediaKind::Video;
    std::int64_t duration_ms = 0;
    std::uint64_t file_size_bytes = 0;
    int width = 0;
    int height = 0;
};

struct Transition {
    TransitionKind kind = TransitionKind::None;
    std::int64_t duration_ms = 0;
};

struct ClipAnimation {
    AnimationKind kind = AnimationKind::None;
    std::int64_t duration_ms = 0;
};

struct Clip {
    std::string id;
    std::string media_id;
    std::int64_t source_in_ms = 0;
    std::int64_t source_out_ms = 0;
    // Position on the project timeline. -1 appends after the track's current end.
    std::int64_t timeline_start_ms = -1;
    Transition incoming_transition;
    ClipAnimation animation;
    std::int64_t fade_in_ms = 0;
    std::int64_t fade_out_ms = 0;
    double volume = 1.0;
};

struct Track {
    std::string id;
    std::string name;
    TrackKind kind = TrackKind::Video;
    bool muted = false;
    bool visible = true;
    std::vector<Clip> clips;
};

class Project {
public:
    explicit Project(std::string name = "Untitled Project");

    const std::string& id() const noexcept;
    const std::string& name() const noexcept;
    void set_name(std::string name);

    const std::vector<MediaAsset>& media() const noexcept;
    const std::vector<Track>& tracks() const noexcept;

    bool add_media(MediaAsset asset);
    bool update_media_info(const std::string& media_id, std::int64_t duration_ms,
                           int width, int height);
    bool add_track(Track track);
    bool add_clip(const std::string& track_id, Clip clip);
    bool remove_clip(const std::string& track_id, const std::string& clip_id);
    bool trim_clip(const std::string& track_id, const std::string& clip_id,
                   std::int64_t source_in_ms, std::int64_t source_out_ms);
    bool set_transition(const std::string& track_id, const std::string& clip_id,
                        Transition transition);
    bool set_clip_volume(const std::string& track_id, const std::string& clip_id,
                         double volume);
    bool set_clip_animation(const std::string& track_id, const std::string& clip_id,
                            ClipAnimation animation);
    bool set_clip_fades(const std::string& track_id, const std::string& clip_id,
                        std::int64_t fade_in_ms, std::int64_t fade_out_ms);
    std::optional<std::string> split_clip(const std::string& track_id,
                                           const std::string& clip_id,
                                           std::int64_t split_source_time_ms);
    bool move_clip(const std::string& track_id, const std::string& clip_id,
                   std::size_t target_index);
    bool move_clip_to_time(const std::string& track_id, const std::string& clip_id,
                           std::int64_t timeline_start_ms);

    std::int64_t duration_ms() const noexcept;
    bool validate(std::string* error = nullptr) const;

private:
    Track* find_track(const std::string& id) noexcept;
    const Track* find_track(const std::string& id) const noexcept;
    Clip* find_clip(Track& track, const std::string& id) noexcept;
    const MediaAsset* find_media(const std::string& id) const noexcept;

    std::string id_;
    std::string name_;
    std::vector<MediaAsset> media_;
    std::vector<Track> tracks_;
    std::uint64_t next_id_ = 1;
};

} // namespace vecut
