#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace vecut {
struct TimelineClip {
    int id;
    std::string uri;
    std::string mime_type;
    int64_t source_in_ms;
    int64_t source_out_ms;
};

class Timeline {
public:
    int add_clip(const std::string& uri, const std::string& mime_type);
    bool remove_clip(int id);
    bool trim_clip(int id, int64_t source_in_ms, int64_t source_out_ms);
    bool set_clip_duration(int id, int64_t duration_ms);
    bool move_clip(int id, int target_index);
    int split_clip(int id, int64_t split_at_ms);
    void clear();
    int clip_count() const;
    std::string summary() const;
private:
    mutable std::mutex mutex_;
    std::vector<TimelineClip> clips_;
    int next_id_ = 1;
};
}  // namespace vecut
