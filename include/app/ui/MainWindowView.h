#pragma once

#include <functional>
#include <memory>
#include <string_view>

#include <app/ImGuiLayer.h>
#include <app/ui/callbacks/SettingsCallbacks.h>
#include <app/ui/callbacks/TaskCallbacks.h>
#include <app/ui/callbacks/TimerCallbacks.h>
#include <app/ui/callbacks/WindowCallbacks.h>
#include <core/Task.h>
#include <core/Timer.h>
#include <core/WellnessTimer.h>
#include <system/MainWindow.h>
#include <ui/AppState.h>
#include <ui/NavigationTabs.h>

namespace WorkBalance::App::UI {

// Forward declarations
class WaterReminderView;
class StandupReminderView;
class EyeCareReminderView;

/// @brief Combined callbacks for MainWindowView (preserves backward compatibility)
/// @deprecated Use individual callback structs (TimerCallbacks, TaskCallbacks, etc.) instead
struct MainWindowCallbacks {
    std::function<void()> onToggleTimer;
    std::function<void(Core::TimerMode)> onModeChange;
    std::function<void()> onToggleOverlayMode;
    std::function<void()> onRequestClose;
    std::function<void(int pomodoroMinutes, int shortBreakMinutes, int longBreakMinutes)> onDurationsApplied;
    std::function<void(int waterIntervalMins, int waterGoal, int standupIntervalMins, int standupDurationMins,
                       int eyeIntervalMins, int eyeBreakSecs)>
        onWellnessSettingsApplied;
    std::function<void(std::string_view name, int estimated)> onTaskAdded;
    std::function<void(size_t index)> onTaskRemoved;
    std::function<void(size_t index, std::string_view name, int estimated, int completed)> onTaskUpdated;
    std::function<void(size_t index)> onTaskCompletionToggled;
    // Navigation callback
    std::function<void(WorkBalance::NavigationTab)> onTabChanged;

    /// @brief Create MainWindowCallbacks from individual callback structs
    static MainWindowCallbacks fromSplit(TimerCallbacks timer, TaskCallbacks task, SettingsCallbacks settings,
                                         WindowCallbacks window) {
        return MainWindowCallbacks{.onToggleTimer = std::move(timer.onToggle),
                                   .onModeChange = std::move(timer.onModeChange),
                                   .onToggleOverlayMode = std::move(window.onToggleOverlayMode),
                                   .onRequestClose = std::move(window.onRequestClose),
                                   .onDurationsApplied = std::move(settings.onPomodoroDurationsApplied),
                                   .onWellnessSettingsApplied = std::move(settings.onWellnessSettingsApplied),
                                   .onTaskAdded = std::move(task.onAdd),
                                   .onTaskRemoved = std::move(task.onRemove),
                                   .onTaskUpdated = std::move(task.onUpdate),
                                   .onTaskCompletionToggled = std::move(task.onToggleCompletion),
                                   .onTabChanged = std::move(window.onTabChanged)};
    }
};

/// @brief Callbacks for wellness timer interactions
struct WellnessCallbacks {
    std::function<void()> onWaterToggle;
    std::function<void()> onWaterAcknowledge;
    std::function<void()> onWaterResetDaily;

    std::function<void()> onStandupToggle;
    std::function<void()> onStandupAcknowledge;
    std::function<void()> onStandupStartBreak;
    std::function<void()> onStandupEndBreak;

    std::function<void()> onEyeCareToggle;
    std::function<void()> onEyeCareAcknowledge;
    std::function<void()> onEyeCareStartBreak;
    std::function<void()> onEyeCareEndBreak;
};

class MainWindowView {
  public:
    MainWindowView(System::MainWindow& window, App::ImGuiLayer& imgui, Core::Timer& timer,
                   Core::TaskManager& taskManager, AppState& state, MainWindowCallbacks callbacks);

    /// @brief Sets wellness timers for the view to display
    void setWellnessTimers(Core::WellnessTimer* water, Core::WellnessTimer* standup, Core::WellnessTimer* eyeCare);

    /// @brief Sets callbacks for wellness timer interactions
    void setWellnessCallbacks(WellnessCallbacks callbacks);

    void render();

  private:
    void renderOverlayMode();
    void renderHeader();
    void renderNavigationTabs(const ImVec2& window_pos, const ImVec2& window_size, float header_height);
    void renderPomodoroContent();
    void renderWaterContent();
    void renderStandupContent();
    void renderEyeCareContent();
    void renderSettingsPopup();
    void renderEditTaskPopup();
    void renderHelpPopup();
    void renderTaskList();
    void renderModeButtons();
    void renderTimer();
    void renderTimerFrame();
    void renderCurrentTask();
    void renderPomodoroCounter() const;
    void handleWindowDragging();

    /// @brief Updates the background color based on the active tab
    void updateBackgroundColor();

    System::MainWindow& m_window;
    App::ImGuiLayer& m_imgui;
    Core::Timer& m_timer;
    Core::TaskManager& m_task_manager;
    AppState& m_state;
    MainWindowCallbacks m_callbacks;
    WellnessCallbacks m_wellness_callbacks;

    // Wellness timer pointers (owned by Application)
    Core::WellnessTimer* m_water_timer = nullptr;
    Core::WellnessTimer* m_standup_timer = nullptr;
    Core::WellnessTimer* m_eye_care_timer = nullptr;

    // Navigation tabs component
    std::unique_ptr<WorkBalance::UI::NavigationTabs> m_navigation_tabs;
};

} // namespace WorkBalance::App::UI
