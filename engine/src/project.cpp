#include "vecut/project.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace vecut {
namespace {
std::string make_id(std::uint64_t value) {
    return "vc-" + std::to_string(value);
}
}

Project::Project(std::string name)
    : id_("project-1"), name_(name.empty() ? "Untitled Project" : std::move(name)) {}

const std::string& Project::id() const noexcept { return id_; }
const std::string& Project::name() const noexcept { return name_; }
void Project::set_name(std::string name) {
    name_ = name.empty() ? "Untitled Project" : std::move(name);
}
const std::vector<MediaAsset>& Project::media() const noexcept { return media_; }
const std::vector<Track>& Project::tracks() const noexcept { return tracks_; }

bool Project::add_media(MediaAsset asset) {
    if (asset.uri.empty() || asset.id.empty() || asset.duration_ms < 0) return false;
    const auto duplicate = std::find_if(media_.begin(), media_.end(),
        [&](const MediaAsset& item) { return item.id == asset.id; });
    if (duplicate != media_.end()) return false;
    media_.push_back(std::move(asset));
    return true;
}

bool Project::update_media_info(const std::string& media_id, std::int64_t duration_ms,
                                int width, int height) {
    if (duration_ms < 0 || width < 0 || height < 0) return false;
    const auto it = std::find_if(media_.begin(), media_.end(),
        [&](const MediaAsset& item) { return item.id == media_id; });
    if (it == media_.end()) return false;
    it->duration_ms = duration_ms;
    it->width = width;
    it->height = height;
    return true;
}

bool Project::add_track(Track track) {
    if (track.id.empty()) track.id = make_id(next_id_++);
    if (find_track(track.id)) return false;
    tracks_.push_back(std::move(track));
    return true;
}

bool Project::add_clip(const std::string& track_id, Clip clip) {
    Track* track = find_track(track_id);
    const MediaAsset* asset = find_media(clip.media_id);
    if (!track || !asset || clip.id.empty() || find_clip(*track, clip.id)) return false;
    if (clip.source_in_ms < 0 || clip.source_out_ms <= clip.source_in_ms ||
        clip.source_out_ms > asset->duration_ms) return false;
    if (track->kind == TrackKind::Audio && asset->kind == MediaKind::Image) return false;
    if (clip.incoming_transition.duration_ms < 0 ||
        clip.incoming_transition.duration_ms > clip.source_out_ms - clip.source_in_ms) return false;
    track->clips.push_back(std::move(clip));
    return true;
}

bool Project::remove_clip(const std::string& track_id, const std::string& clip_id) {
    Track* track = find_track(track_id);
    if (!track) return false;
    const auto old_size = track->clips.size();
    track->clips.erase(std::remove_if(track->clips.begin(), track->clips.end(),
        [&](const Clip& clip) { return clip.id == clip_id; }), track->clips.end());
    return track->clips.size() != old_size;
}

bool Project::trim_clip(const std::string& track_id, const std::string& clip_id,
                        std::int64_t source_in_ms, std::int64_t source_out_ms) {
    Track* track = find_track(track_id);
    if (!track) return false;
    Clip* clip = find_clip(*track, clip_id);
    if (!clip) return false;
    const MediaAsset* asset = find_media(clip->media_id);
    if (!asset || source_in_ms < 0 || source_out_ms <= source_in_ms ||
        source_out_ms > asset->duration_ms) return false;
    if (clip->incoming_transition.duration_ms > source_out_ms - source_in_ms) return false;
    clip->source_in_ms = source_in_ms;
    clip->source_out_ms = source_out_ms;
    return true;
}

bool Project::set_transition(const std::string& track_id, const std::string& clip_id,
                             Transition transition) {
    Track* track = find_track(track_id);
    if (!track || transition.duration_ms < 0) return false;
    Clip* clip = find_clip(*track, clip_id);
    if (!clip) return false;
    const auto clip_duration = clip->source_out_ms - clip->source_in_ms;
    if (transition.kind == TransitionKind::None) transition.duration_ms = 0;
    if (transition.duration_ms > clip_duration) return false;
    clip->incoming_transition = transition;
    return true;
}

std::optional<std::string> Project::split_clip(const std::string& track_id,
                                                const std::string& clip_id,
                                                std::int64_t split_source_time_ms) {
    Track* track = find_track(track_id);
    if (!track) return std::nullopt;
    const auto it = std::find_if(track->clips.begin(), track->clips.end(),
        [&](const Clip& clip) { return clip.id == clip_id; });
    if (it == track->clips.end() || split_source_time_ms <= it->source_in_ms ||
        split_source_time_ms >= it->source_out_ms) return std::nullopt;

    Clip right = *it;
    right.id = make_id(next_id_++);
    right.source_in_ms = split_source_time_ms;
    right.incoming_transition = {};
    it->source_out_ms = split_source_time_ms;
    const std::string new_id = right.id;
    track->clips.insert(std::next(it), std::move(right));
    return new_id;
}

bool Project::move_clip(const std::string& track_id, const std::string& clip_id,
                        std::size_t target_index) {
    Track* track = find_track(track_id);
    if (!track || target_index >= track->clips.size()) return false;
    const auto it = std::find_if(track->clips.begin(), track->clips.end(),
        [&](const Clip& clip) { return clip.id == clip_id; });
    if (it == track->clips.end()) return false;

    const auto current_index = static_cast<std::size_t>(std::distance(track->clips.begin(), it));
    if (current_index == target_index) return true;

    Clip moving = std::move(*it);
    track->clips.erase(it);
    // target_index is the desired final index in the original ordering. If the
    // clip was last, erasing it makes the new end equal to target_index.
    const auto insertion_index = std::min(target_index, track->clips.size());
    track->clips.insert(track->clips.begin() + static_cast<std::ptrdiff_t>(insertion_index),
                        std::move(moving));
    return true;
}

std::int64_t Project::duration_ms() const noexcept {
    std::int64_t longest = 0;
    for (const auto& track : tracks_) {
        std::int64_t position = 0;
        for (const auto& clip : track.clips) {
            position += clip.source_out_ms - clip.source_in_ms;
        }
        longest = std::max(longest, position);
    }
    return longest;
}

bool Project::validate(std::string* error) const {
    auto fail = [&](const char* message) {
        if (error) *error = message;
        return false;
    };
    for (const auto& track : tracks_) {
        for (const auto& clip : track.clips) {
            const auto* asset = find_media(clip.media_id);
            if (!asset) return fail("A clip refers to missing media.");
            if (clip.source_in_ms < 0 || clip.source_out_ms <= clip.source_in_ms ||
                clip.source_out_ms > asset->duration_ms)
                return fail("A clip has an invalid source range.");
            if (!std::isfinite(clip.volume) || clip.volume < 0.0 || clip.volume > 4.0)
                return fail("A clip volume must be between 0 and 4.");
            if (clip.incoming_transition.duration_ms < 0 ||
                clip.incoming_transition.duration_ms > clip.source_out_ms - clip.source_in_ms)
                return fail("A transition is longer than its clip.");
        }
    }
    if (error) error->clear();
    return true;
}

Track* Project::find_track(const std::string& id) noexcept {
    const auto it = std::find_if(tracks_.begin(), tracks_.end(),
        [&](const Track& track) { return track.id == id; });
    return it == tracks_.end() ? nullptr : &*it;
}
const Track* Project::find_track(const std::string& id) const noexcept {
    const auto it = std::find_if(tracks_.begin(), tracks_.end(),
        [&](const Track& track) { return track.id == id; });
    return it == tracks_.end() ? nullptr : &*it;
}
Clip* Project::find_clip(Track& track, const std::string& id) noexcept {
    const auto it = std::find_if(track.clips.begin(), track.clips.end(),
        [&](const Clip& clip) { return clip.id == id; });
    return it == track.clips.end() ? nullptr : &*it;
}
const MediaAsset* Project::find_media(const std::string& id) const noexcept {
    const auto it = std::find_if(media_.begin(), media_.end(),
        [&](const MediaAsset& asset) { return asset.id == id; });
    return it == media_.end() ? nullptr : &*it;
}

} // namespace vecut
