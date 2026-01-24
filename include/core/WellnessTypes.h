#pragma once

#include <imgui.h>
#include <string_view>

namespace WorkBalance::Core {

/// @brief Types of wellness reminders available in the app
enum class WellnessType {
    Pomodoro, ///< Focus/work timer with breaks
    Water,    ///< Hydration reminders
    Standup,  ///< Movement/standing reminders
    EyeStrain ///< 20-20-20 rule eye care reminders
};

/// @brief Configuration for wellness reminder defaults
struct WellnessDefaults {
    // Water reminder defaults (in seconds)
    static constexpr int DEFAULT_WATER_INTERVAL = 30 * 60; // 30 minutes
    static constexpr int WATER_INTERVAL_MIN = 15 * 60;     // 15 minutes
    static constexpr int WATER_INTERVAL_MAX = 120 * 60;    // 2 hours
    static constexpr int DEFAULT_DAILY_WATER_GOAL = 8;     // 8 glasses

    // Standup reminder defaults (in seconds)
    static constexpr int DEFAULT_STANDUP_INTERVAL = 45 * 60; // 45 minutes
    static constexpr int STANDUP_INTERVAL_MIN = 15 * 60;     // 15 minutes
    static constexpr int STANDUP_INTERVAL_MAX = 120 * 60;    // 2 hours
    static constexpr int DEFAULT_STANDUP_DURATION = 5 * 60;  // 5 minutes standing

    // Eye strain (20-20-20 rule) defaults (in seconds)
    static constexpr int DEFAULT_EYE_INTERVAL = 20 * 60;  // 20 minutes
    static constexpr int EYE_INTERVAL_MIN = 10 * 60;      // 10 minutes
    static constexpr int EYE_INTERVAL_MAX = 60 * 60;      // 60 minutes
    static constexpr int DEFAULT_EYE_BREAK_DURATION = 20; // 20 seconds looking away

    // Theme colors for each wellness type
    static constexpr ImVec4 WATER_BG_COLOR{0.20f, 0.50f, 0.70f, 1.0f};      // Blue
    static constexpr ImVec4 STANDUP_BG_COLOR{0.55f, 0.35f, 0.60f, 1.0f};    // Purple
    static constexpr ImVec4 EYE_STRAIN_BG_COLOR{0.25f, 0.55f, 0.45f, 1.0f}; // Teal/Green
};

/// @brief State for water tracking
struct WaterState {
    int interval_seconds = WellnessDefaults::DEFAULT_WATER_INTERVAL;
    int daily_goal = WellnessDefaults::DEFAULT_DAILY_WATER_GOAL;
    int glasses_consumed = 0;
    int remaining_time = WellnessDefaults::DEFAULT_WATER_INTERVAL;
    bool timer_running = false;
    bool reminder_active = false;
};

/// @brief State for standup tracking
struct StandupState {
    int interval_seconds = WellnessDefaults::DEFAULT_STANDUP_INTERVAL;
    int break_duration_seconds = WellnessDefaults::DEFAULT_STANDUP_DURATION;
    int standups_completed = 0;
    int remaining_time = WellnessDefaults::DEFAULT_STANDUP_INTERVAL;
    bool timer_running = false;
    bool in_standup_break = false; // True when user should be standing
    bool reminder_active = false;
};

/// @brief State for eye strain (20-20-20 rule) tracking
struct EyeStrainState {
    int interval_seconds = WellnessDefaults::DEFAULT_EYE_INTERVAL;
    int break_duration_seconds = WellnessDefaults::DEFAULT_EYE_BREAK_DURATION;
    int breaks_completed = 0;
    int remaining_time = WellnessDefaults::DEFAULT_EYE_INTERVAL;
    bool timer_running = false;
    bool in_eye_break = false; // True when user should be looking away
    bool reminder_active = false;
};

/// @brief Get the display name for a wellness type
[[nodiscard]] constexpr std::string_view getWellnessTypeName(WellnessType type) noexcept {
    switch (type) {
        case WellnessType::Pomodoro:
            return "Pomodoro";
        case WellnessType::Water:
            return "Hydration";
        case WellnessType::Standup:
            return "Stand Up";
        case WellnessType::EyeStrain:
            return "Eye Care";
        default:
            return "Unknown";
    }
}

/// @brief Get the background color for a wellness type
[[nodiscard]] constexpr ImVec4 getWellnessColor(WellnessType type) noexcept {
    switch (type) {
        case WellnessType::Water:
            return WellnessDefaults::WATER_BG_COLOR;
        case WellnessType::Standup:
            return WellnessDefaults::STANDUP_BG_COLOR;
        case WellnessType::EyeStrain:
            return WellnessDefaults::EYE_STRAIN_BG_COLOR;
        default:
            return ImVec4{0.85f, 0.35f, 0.35f, 1.0f}; // Pomodoro red
    }
}

} // namespace WorkBalance::Core
