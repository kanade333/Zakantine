// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cstddef>
#include <optional>
#include <vector>

#include "common/common_types.h"

namespace Vulkan {

// Which graphics pipelines are destroyed to bound driver memory (~0.45 MB each).
// By default none are while the cache holds at most `cap` pipelines: destroying one that a later
// area uses again rebuilds it on the GPU thread (in Ocarina of Time, ~3x more runtime builds once
// age-based eviction started). Above the cap, the least recently used are destroyed down to 90%
// of it, never one used in the last `min_idle` frames.
// EDEN_NX2_PIPELINE_CAP overrides the cap (0: never evict). EDEN_NX2_PIPELINE_EVICT_FRAMES
// restores the previous rule instead: destroy every pipeline unused for that many frames
// (9000 upstream; 0: never evict).
struct EvictionPolicy {
    u64 cap = 6000;
    u64 min_idle = 600;
    std::optional<u64> age;
};

/// Pipelines last used before `below` are evicted, and `ties` of those last used at `tie_frame`.
struct EvictionSelection {
    u64 below{};
    u64 tie_frame{};
    size_t ties{};
};

/// `last_uses` holds the last use of every evictable pipeline (reordered); nullopt evicts none.
inline std::optional<EvictionSelection> SelectEvictions(const EvictionPolicy& policy,
                                                        u64 frame_count, size_t cache_size,
                                                        std::vector<u64>& last_uses) {
    if (policy.age) {
        if (*policy.age == 0 || frame_count <= *policy.age) {
            return std::nullopt;
        }
        return EvictionSelection{.below = frame_count - *policy.age};
    }
    if (policy.cap == 0 || cache_size <= policy.cap || frame_count <= policy.min_idle) {
        return std::nullopt;
    }
    const u64 idle_cutoff = frame_count - policy.min_idle;
    std::erase_if(last_uses, [idle_cutoff](u64 last_use) { return last_use >= idle_cutoff; });
    const size_t target = static_cast<size_t>(policy.cap - policy.cap / 10);
    const size_t count = std::min(cache_size - target, last_uses.size());
    if (count == 0) {
        return std::nullopt;
    }
    if (count == last_uses.size()) {
        return EvictionSelection{.below = idle_cutoff};
    }
    // The count oldest: all last used before the count-th oldest use, plus enough at that use.
    const auto nth = last_uses.begin() + static_cast<std::ptrdiff_t>(count - 1);
    std::nth_element(last_uses.begin(), nth, last_uses.end());
    const u64 tie_frame = *nth;
    const size_t older = static_cast<size_t>(
        std::count_if(last_uses.begin(), nth, [tie_frame](u64 use) { return use < tie_frame; }));
    return EvictionSelection{.below = tie_frame, .tie_frame = tie_frame, .ties = count - older};
}

} // namespace Vulkan
