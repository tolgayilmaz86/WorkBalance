#pragma once

#include <core/Timer.h>

#include <functional>

namespace WorkBalance::App::UI {

/// @brief Callbacks for timer-related operations
struct TimerCallbacks {
    std::function<void()> onToggle;
    std::function<void(Core::TimerMode)> onModeChange;
    std::function<void()> onReset;
};

} // namespace WorkBalance::App::UI
