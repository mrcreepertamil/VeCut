#include "timeline.h"
#include <algorithm>
#include <sstream>

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
    it->source_in_ms = source_in_ms;
    it->source_out_ms = source_out_ms;
    return true;
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
    for (const TimelineClip& clip : clips_) {
        output << "\n#" << clip.id << "  "
               << (clip.mime_type.empty() ? "unknown media" : clip.mime_type);
        if (clip.source_in_ms > 0 || clip.source_out_ms >= 0) {
            output << "  [" << clip.source_in_ms << "–";
            if (clip.source_out_ms < 0) output << "end";
            else output << clip.source_out_ms;
            output << " ms]";
        }
    }
    return output.str();
}
}  // namespace vecut
