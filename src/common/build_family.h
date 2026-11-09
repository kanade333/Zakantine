// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string_view>

#include "common/scm_rev.h"

namespace Common {

/// Fritchii (Frizti's fork of Lichii) and the forks built on it, such as Zakantine: builds tuned
/// for Ocarina of Time that use Fritchii's interface, shader preparation and frame pacing.
[[nodiscard]] inline bool IsFritchiiFamily() {
    const std::string_view name{g_build_name};
    return name == "Fritchii" || name == "Zakantine";
}

/// Zakantine (kanade333/Zakantine), a fork of Fritchii.
[[nodiscard]] inline bool IsZakantine() {
    return std::string_view{g_build_name} == "Zakantine";
}

} // namespace Common
