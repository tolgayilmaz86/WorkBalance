#pragma once

#include <functional>

#include <app/ImGuiLayer.h>
#include <core/Timer.h>
#include <ui/AppState.h>

namespace WorkBalance::App::UI::Components {

/// @brief Self-contained component for rendering the timer display and controls
/// @details Handles mode selection buttons, timer display, and start/pause button
class TimerPanel {
  public:
    /// @brief Callbacks for timer interactions
    struct Callbacks {
        std::function<void()> onToggle;
        std::function<void(Core::TimerMode)> onModeChange;
    };

    /// @brief Constructs the timer panel component
    /// @param timer Reference to the timer for reading state
    /// @param state Reference to the application state
    /// @param imgui Reference to ImGui layer for fonts
    /// @param callbacks Callbacks to invoke on user interactions
    TimerPanel(const Core::Timer& timer, AppState& state, App::ImGuiLayer& imgui, Callbacks callbacks);

    /// @brief Renders the complete timer panel (frame with mode buttons and timer)
    void render();

    /// @brief Renders just the timer display and button (without the frame)
    void renderTimerOnly();

  private:
    /// @brief Renders the mode selection buttons (Pomodoro, Short Break, Long Break)
    void renderModeButtons();

    /// @brief Renders the timer display and start/pause button
    void renderTimerDisplay();

    const Core::Timer& m_timer;
    AppState& m_state;
    App::ImGuiLayer& m_imgui;
    Callbacks m_callbacks;
};

} // namespace WorkBalance::App::UI::Components
