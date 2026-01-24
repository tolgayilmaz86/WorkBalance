#pragma once

#include <functional>
#include <core/WellnessTimer.h>
#include <core/WellnessTypes.h>
#include <app/ImGuiLayer.h>
#include <ui/AppState.h>
#include <imgui.h>

namespace WorkBalance::App::UI {

/// @brief Callbacks for wellness reminder interactions
struct WellnessViewCallbacks {
    std::function<void()> onToggleTimer;
    std::function<void()> onAcknowledge;  // User acknowledged reminder
    std::function<void()> onStartBreak;   // Start break (standup/eye care)
    std::function<void()> onEndBreak;     // End break early
    std::function<void()> onResetDaily;   // Reset daily counters
    std::function<void()> onOpenSettings; // Open settings popup
};

/// @brief Renders the water hydration reminder view
class WaterReminderView {
  public:
    WaterReminderView(App::ImGuiLayer& imgui, Core::WellnessTimer& timer, AppState& state,
                      WellnessViewCallbacks callbacks);

    void render();

  private:
    void renderTimer();
    void renderProgress();
    void renderControls();
    void renderGoalTracker();

    App::ImGuiLayer& m_imgui;
    Core::WellnessTimer& m_timer;
    AppState& m_state;
    WellnessViewCallbacks m_callbacks;
};

/// @brief Renders the stand up reminder view
class StandupReminderView {
  public:
    StandupReminderView(App::ImGuiLayer& imgui, Core::WellnessTimer& timer, AppState& state,
                        WellnessViewCallbacks callbacks);

    void render();

  private:
    void renderTimer();
    void renderBreakMode();
    void renderControls();
    void renderStats();

    App::ImGuiLayer& m_imgui;
    Core::WellnessTimer& m_timer;
    AppState& m_state;
    WellnessViewCallbacks m_callbacks;
};

/// @brief Renders the eye care (20-20-20 rule) view
class EyeCareReminderView {
  public:
    EyeCareReminderView(App::ImGuiLayer& imgui, Core::WellnessTimer& timer, AppState& state,
                        WellnessViewCallbacks callbacks);

    void render();

  private:
    void renderTimer();
    void renderBreakMode();
    void renderControls();
    void renderStats();
    void renderTip();

    App::ImGuiLayer& m_imgui;
    Core::WellnessTimer& m_timer;
    AppState& m_state;
    WellnessViewCallbacks m_callbacks;
};

} // namespace WorkBalance::App::UI
