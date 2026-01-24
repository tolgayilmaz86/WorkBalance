#pragma once

#include "../../core/Configuration.h"

namespace WorkBalance {

/// @brief Runtime counters for tracking progress (not editing state)
struct RuntimeCounters {
    // Pomodoro counters cached from domain layer
    int target_pomodoros = 0;
    int completed_pomodoros = 0;

    // Wellness counters cached from domain layer
    int water_glasses_consumed = 0;
    int water_daily_goal = Core::Configuration::DEFAULT_WATER_DAILY_GOAL;
    int standups_completed = 0;
    int eye_breaks_completed = 0;
};

} // namespace WorkBalance
