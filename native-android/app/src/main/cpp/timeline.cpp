#include "timeline.h"
#include <cstddef>
#include <algorithm>
#include <sstream>
#include <utility>

namespace vecut {
int Timeline::add_clip(const std::string& uri, const std::string& mime_type) {
    if (uri.empty()) return -1;
    std::lock_guard<std::mutex> lock(mutex_);
    const int id = next_id_++;
    clips_.push_back(TimelineClip{id, uri, mime_type, 0, -1});
    return id;
}

bool Timeline::remove_clip(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto old_size = clips_.size();
    clips_.erase(std::remove_if(clips_.begin(), clips_.end(),
        [id](const TimelineClip& clip) { return clip.id == id; }), clips_.end());
    return clips_.size() != old_size;
}

bool Timeline::trim_clip(int id, int64_t source_in_ms, int64_t source_out_ms) {
    if (source_in_ms < 0 || source_out_ms <= source_in_ms) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = std::find_if(clips_.begin(), clips_.end(),
        [id](const TimelineClip& clip) { return clip.id == id; });
    if (it == clips_.end()) return false;
    if (it->source_out_ms >= 0 && source_out_ms > it->source_out_ms) return false;
    it->source_in_ms = source_in_ms;
    it->source_out_ms = source_out_ms;
    return true;
}

bool Timeline::set_transition(int id, int type, int64_t duration_ms) {
    if (type < 0 || type > 6 || duration_ms < 0 || duration_ms > 10000) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = std::find_if(clips_.begin(), clips_.end(),
        [id](const TimelineClip& clip) { return clip.id == id; });
    if (it == clips_.end()) return false;
    if (type == 0) {
        it->transition_type = 0;
        it->transition_duration_ms = 0;
    } else {
        if (duration_ms < 100) return false;
        if (it->source_out_ms >= 0 &&
            duration_ms > (it->source_out_ms - it->source_in_ms)) return false;
        it->transition_type = type;
        it->transition_duration_ms = duration_ms;
    }
    return true;
}

bool Timeline::get_clip_settings(int id, int64_t* source_in_ms, int64_t* source_out_ms,
                                 int* transition_type, int64_t* transition_duration_ms) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = std::find_if(clips_.begin(), clips_.end(),
        [id](const TimelineClip& clip) { return clip.id == id; });
    if (it == clips_.end()) return false;
    if (source_in_ms) *source_in_ms = it->source_in_ms;
    if (source_out_ms) *source_out_ms = it->source_out_ms;
    if (transition_type) *transition_type = it->transition_type;
    if (transition_duration_ms) *transition_duration_ms = it->transition_duration_ms;
    return true;
}

bool Timeline::set_clip_duration(int id, int64_t duration_ms) {
    if (duration_ms <= 0) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = std::find_if(clips_.begin(), clips_.end(),
        [id](const TimelineClip& clip) { return clip.id == id; });
    if (it == clips_.end()) return false;
    if (it->source_in_ms >= duration_ms) return false;
    if (it->source_out_ms < 0 || it->source_out_ms > duration_ms) {
        it->source_out_ms = duration_ms;
    }
    return true;
}

bool Timeline::move_clip(int id, int target_index) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = std::find_if(clips_.begin(), clips_.end(),
        [id](const TimelineClip& clip) { return clip.id == id; });
    if (it == clips_.end()) return false;
    if (target_index < 0 || target_index >= static_cast<int>(clips_.size())) return false;
    const int current_index = static_cast<int>(std::distance(clips_.begin(), it));
    if (current_index == target_index) return true;
    TimelineClip moving = *it;
    clips_.erase(it);
    clips_.insert(clips_.begin() + target_index, std::move(moving));
    return true;
}

int Timeline::split_clip(int id, int64_t split_at_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = std::find_if(clips_.begin(), clips_.end(),
        [id](const TimelineClip& clip) { return clip.id == id; });
    if (it == clips_.end()) return -1;
    if (it->source_out_ms < 0 || split_at_ms <= it->source_in_ms ||
        split_at_ms >= it->source_out_ms) return -1;

    const auto current_index = static_cast<std::size_t>(std::distance(clips_.begin(), it));
    TimelineClip right = *it;
    right.id = next_id_++;
    right.source_in_ms = split_at_ms;
    it->source_out_ms = split_at_ms;
    const int new_id = right.id;
    clips_.insert(clips_.begin() + static_cast<std::ptrdiff_t>(current_index + 1),
                  std::move(right));
    return new_id;
}

void Timeline::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    clips_.clear();
}

int Timeline::clip_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int>(clips_.size());
}

std::string Timeline::summary() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream output;
    output << "Timeline · " << clips_.size()
           << (clips_.size() == 1 ? " clip" : " clips");
    int64_t timeline_position_ms = 0;
    for (std::size_t index = 0; index < clips_.size(); ++index) {
        const TimelineClip& clip = clips_[index];
        output << "\n" << index << ". #" << clip.id << "  "
               << (clip.mime_type.empty() ? "unknown media" : clip.mime_type);
        if (clip.source_out_ms >= 0) {
            const int64_t duration = clip.source_out_ms - clip.source_in_ms;
            output << "  [" << clip.source_in_ms << "–"
                   << clip.source_out_ms << " ms; " << duration << " ms]";
            timeline_position_ms += duration;
        } else {
            output << "  [duration unknown]";
        }
        if (clip.transition_type > 0) {
            static const char* names[] = {"None", "Fade", "Dissolve", "Wipe L-R", "Wipe R-L", "Slide L-R", "Slide R-L"};
            output << "  {transition=" << names[clip.transition_type]
                   << ", " << clip.transition_duration_ms << " ms}";
        }
    }
    output << "\nTotal known duration: " << timeline_position_ms << " ms";
    return output.str();
}
}  // namespace vecut
