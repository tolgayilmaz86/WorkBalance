#pragma once

#include "../../core/Configuration.h"

namespace WorkBalance {

/// @brief Temporary state for editing Pomodoro timer settings in the settings popup
struct PomodoroSettingsEdit {
    int pomodoro_duration = Core::Configuration::DEFAULT_POMODORO_MINUTES;
    int short_break_duration = Core::Configuration::DEFAULT_SHORT_BREAK_MINUTES;
    int long_break_duration = Core::Configuration::DEFAULT_LONG_BREAK_MINUTES;
};

/// @brief Temporary state for editing water reminder settings
struct WaterSettingsEdit {
    int interval = Core::Configuration::DEFAULT_WATER_INTERVAL_MINUTES;
    int daily_goal = Core::Configuration::DEFAULT_WATER_DAILY_GOAL;
};

/// @brief Temporary state for editing standup reminder settings
struct StandupSettingsEdit {
    int interval = Core::Configuration::DEFAULT_STANDUP_INTERVAL_MINUTES;
    int duration = Core::Configuration::DEFAULT_STANDUP_DURATION_MINUTES;
};

/// @brief Temporary state for editing eye care reminder settings
struct EyeCareSettingsEdit {
    int interval = Core::Configuration::DEFAULT_EYE_INTERVAL_MINUTES;
    int break_duration = Core::Configuration::DEFAULT_EYE_BREAK_DURATION_SECONDS;
};

/// @brief Aggregated settings edit state for all wellness features
struct WellnessSettingsEdit {
    WaterSettingsEdit water;
    StandupSettingsEdit standup;
    EyeCareSettingsEdit eye_care;
};

} // namespace WorkBalance
