#pragma once

#include <functional>

namespace WorkBalance::App::UI {

/// @brief Callbacks for settings-related operations
struct SettingsCallbacks {
    std::function<void(int pomodoroMinutes, int shortBreakMinutes, int longBreakMinutes)> onPomodoroDurationsApplied;
    std::function<void(int waterIntervalMins, int waterGoal, int standupIntervalMins, int standupDurationMins,
                       int eyeIntervalMins, int eyeBreakSecs)>
        onWellnessSettingsApplied;
};

} // namespace WorkBalance::App::UI
