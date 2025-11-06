#pragma once

#include <functional>
#include <string_view>

#include <app/ImGuiLayer.h>
#include <core/Task.h>
#include <core/Timer.h>
#include <system/MainWindow.h>
#include <ui/AppState.h>

namespace WorkBalance::App::UI {
struct MainWindowCallbacks {
    std::function<void()> onToggleTimer;
    std::function<void(Core::TimerMode)> onModeChange;
    std::function<void()> onToggleOverlayMode;
    std::function<void()> onRequestClose;
    std::function<void(int pomodoroMinutes, int shortBreakMinutes, int longBreakMinutes)> onDurationsApplied;
    std::function<void(std::string_view name, int estimated)> onTaskAdded;
    std::function<void(size_t index)> onTaskRemoved;
    std::function<void(size_t index, std::string_view name, int estimated, int completed)> onTaskUpdated;
    std::function<void(size_t index)> onTaskCompletionToggled;
};

class MainWindowView {
  public:
    MainWindowView(System::MainWindow& window, App::ImGuiLayer& imgui, Core::Timer& timer,
                   Core::TaskManager& taskManager, AppState& state, MainWindowCallbacks callbacks);

    void render();

  private:
    void renderOverlayMode();
    void renderHeader();
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

    System::MainWindow& m_window;
    App::ImGuiLayer& m_imgui;
    Core::Timer& m_timer;
    Core::TaskManager& m_task_manager;
    AppState& m_state;
    MainWindowCallbacks m_callbacks;
};

} // namespace WorkBalance::App::UI
