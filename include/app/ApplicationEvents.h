#pragma once

#include <core/Event.h>
#include <core/Observable.h>
#include <core/Timer.h>
#include <core/WellnessTypes.h>
#include <imgui.h>

namespace WorkBalance::App {

/// @brief Application-wide events for decoupled component communication
///
/// This class provides a centralized event bus for the application.
/// Components can subscribe to events without direct coupling to each other.
///
/// Example usage:
/// @code
/// ApplicationEvents& events = getAppEvents();
///
/// // Subscribe to timer completion
/// events.onTimerComplete.subscribe([](Core::TimerMode mode) {
///     showNotification("Timer complete!");
/// });
///
/// // Subscribe to tab changes
/// events.onActiveTabChanged.subscribe([](NavigationTab tab) {
///     updateBackgroundColor(tab);
/// });
/// @endcode
class ApplicationEvents {
  public:
    // ===== Timer Events =====

    /// @brief Emitted when a pomodoro/break timer completes
    Core::Event<Core::TimerMode> onTimerComplete;

    /// @brief Emitted when the timer mode changes (Pomodoro/ShortBreak/LongBreak)
    Core::Event<Core::TimerMode> onTimerModeChanged;

    /// @brief Emitted every second while timer is running
    Core::Event<int> onTimerTick;

    /// @brief Emitted when timer starts or pauses
    Core::Event<bool> onTimerRunningChanged; // true = started, false = paused

    // ===== Wellness Events =====

    /// @brief Emitted when a wellness timer completes (water/standup/eye care)
    Core::Event<Core::WellnessType> onWellnessTimerComplete;

    /// @brief Emitted when a wellness break starts
    Core::Event<Core::WellnessType> onWellnessBreakStarted;

    /// @brief Emitted when a wellness break ends
    Core::Event<Core::WellnessType> onWellnessBreakEnded;

    /// @brief Emitted when wellness activity is acknowledged
    Core::Event<Core::WellnessType> onWellnessAcknowledged;

    // ===== Task Events =====

    /// @brief Emitted when tasks are added, removed, or modified
    Core::Event<> onTasksChanged;

    /// @brief Emitted when a pomodoro is completed for a task
    Core::Event<std::size_t> onTaskPomodoroCompleted; // task index

    // ===== UI Events =====

    /// @brief Emitted when overlay visibility changes
    Core::Event<bool> onOverlayVisibilityChanged;

    /// @brief Emitted when settings popup opens/closes
    Core::Event<bool> onSettingsVisibilityChanged;

    /// @brief Emitted when the application requests to close
    Core::Event<> onCloseRequested;

    // ===== Navigation Events =====

    /// @brief Emitted when the active tab changes
    Core::Event<int> onActiveTabChanged; // NavigationTab as int for header independence
};

/// @brief Application-wide observable state
///
/// This provides reactive state that automatically notifies observers when values change.
/// Useful for state that multiple components need to display or react to.
///
/// Example usage:
/// @code
/// ApplicationState& state = getAppState();
///
/// // Observe background color changes
/// state.backgroundColor.observe([](const ImVec4& old_color, const ImVec4& new_color) {
///     updateTheme(new_color);
/// });
///
/// // Read current value
/// ImVec4 bg = state.backgroundColor.get();
///
/// // Update value (notifies observers if changed)
/// state.backgroundColor.set(newColor);
/// @endcode
class ApplicationState {
  public:
    // ===== UI State =====

    /// @brief Current background color based on active mode/tab
    Core::Observable<ImVec4> backgroundColor{ImVec4{0.76f, 0.35f, 0.35f, 1.0f}};

    /// @brief Whether the timer overlay is visible
    Core::Observable<bool> showTimerOverlay{false};

    /// @brief Whether the main window is in overlay mode (always on top, transparent)
    Core::Observable<bool> mainWindowOverlayMode{false};

    // ===== Counters =====

    /// @brief Completed pomodoro count across all tasks
    Core::Observable<int> completedPomodoros{0};

    /// @brief Target pomodoro count from all tasks
    Core::Observable<int> targetPomodoros{0};

    /// @brief Water glasses consumed today
    Core::Observable<int> waterGlassesConsumed{0};

    /// @brief Water daily goal
    Core::Observable<int> waterDailyGoal{8};

    /// @brief Standups completed today
    Core::Observable<int> standupsCompleted{0};

    /// @brief Eye breaks completed today
    Core::Observable<int> eyeBreaksCompleted{0};

    // ===== Timer State =====

    /// @brief Current timer remaining time in seconds
    Core::Observable<int> timerRemainingSeconds{0};

    /// @brief Current timer mode
    Core::Observable<Core::TimerMode> currentTimerMode{Core::TimerMode::Pomodoro};

    /// @brief Whether the timer is currently running
    Core::Observable<bool> timerRunning{false};
};

} // namespace WorkBalance::App
