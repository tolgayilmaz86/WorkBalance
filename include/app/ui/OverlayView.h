#pragma once

#include <app/ImGuiLayer.h>
#include <core/Timer.h>
#include <core/WellnessTimer.h>
#include <system/OverlayWindow.h>
#include <ui/AppState.h>

namespace WorkBalance::App::UI {
class OverlayView {
  public:
    OverlayView(App::ImGuiLayer& imgui, Core::Timer& timer, AppState& state)
        : m_imgui(imgui), m_timer(timer), m_state(state) {
    }

    /// @brief Sets the wellness timers for overlay display
    void setWellnessTimers(Core::WellnessTimer* water, Core::WellnessTimer* standup, Core::WellnessTimer* eye_care) {
        m_water_timer = water;
        m_standup_timer = standup;
        m_eye_care_timer = eye_care;
    }

    /// @brief Renders the content of the overlay window.
    /// @param overlay_window The overlay window to render content for.
    void renderContent(System::OverlayWindow& overlay_window);

    /// @brief Renders the overlay window frame.
    /// @param overlay_window The overlay window to render the frame for.
    void renderFrame(System::OverlayWindow& overlay_window);

  private:
    App::ImGuiLayer& m_imgui;
    Core::Timer& m_timer;
    AppState& m_state;
    Core::WellnessTimer* m_water_timer = nullptr;
    Core::WellnessTimer* m_standup_timer = nullptr;
    Core::WellnessTimer* m_eye_care_timer = nullptr;
};

} // namespace WorkBalance::App::UI
