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
    int transition_type = 0;  // Incoming transition: 0 none, 1 fade, 2 dissolve, 3 wipe L/R, 4 wipe R/L, 5 slide L/R, 6 slide R/L.
    int64_t transition_duration_ms = 500;
};

class Timeline {
public:
    int add_clip(const std::string& uri, const std::string& mime_type);
    bool remove_clip(int id);
    bool trim_clip(int id, int64_t source_in_ms, int64_t source_out_ms);
    bool set_transition(int id, int type, int64_t duration_ms);
    bool get_clip_settings(int id, int64_t* source_in_ms, int64_t* source_out_ms,
                           int* transition_type, int64_t* transition_duration_ms) const;
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
