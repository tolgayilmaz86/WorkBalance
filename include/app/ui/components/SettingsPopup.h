#pragma once

#include <functional>
#include <ui/AppState.h>

namespace WorkBalance::App::UI::Components {

/// @brief Self-contained component for rendering the settings popup dialog
/// @details Handles all settings editing including Pomodoro durations, wellness intervals,
///          and overlay visibility options
class SettingsPopup {
  public:
    /// @brief Callbacks for settings changes
    struct Callbacks {
        std::function<void(int pomodoroMinutes, int shortBreakMinutes, int longBreakMinutes)>
            onPomodoroDurationsApplied;
        std::function<void(int waterIntervalMins, int waterGoal, int standupIntervalMins, int standupDurationMins,
                           int eyeIntervalMins, int eyeBreakSecs)>
            onWellnessSettingsApplied;
    };

    /// @brief Constructs the settings popup component
    /// @param state Reference to the application state (for temp values and flags)
    /// @param callbacks Callbacks to invoke when settings are saved
    SettingsPopup(AppState& state, Callbacks callbacks);

    /// @brief Renders the settings popup if triggered
    void render();

  private:
    /// @brief Renders the Pomodoro timer duration settings section
    void renderPomodoroSection();

    /// @brief Renders the wellness reminders settings section
    void renderWellnessSection();

    /// @brief Renders the startup settings section
    void renderStartupSection();

    /// @brief Renders a duration input row with +/- buttons
    /// @param label The label for the row
    /// @param minusId ImGui ID for minus button
    /// @param inputId ImGui ID for input field
    /// @param plusId ImGui ID for plus button
    /// @param value Reference to the value being edited
    /// @param minValue Minimum allowed value
    /// @param maxValue Maximum allowed value
    void renderDurationRow(const char* label, const char* minusId, const char* inputId, const char* plusId, int& value,
                           int minValue, int maxValue);

    AppState& m_state;
    Callbacks m_callbacks;
};

} // namespace WorkBalance::App::UI::Components
