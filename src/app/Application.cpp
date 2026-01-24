#define NOMINMAX
#include <app/Application.h>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include <app/ImGuiLayer.h>
#include <app/ui/MainWindowView.h>
#include <app/ui/OverlayView.h>
#include <core/Configuration.h>
#include <core/Persistence.h>
#include <core/Task.h>
#include <core/Timer.h>
#include <core/WellnessTimer.h>
#include <core/WellnessTypes.h>
#include <system/AudioManager.h>
#include <system/MainWindow.h>
#include <system/OverlayWindow.h>
#include <system/SystemTray.h>
#include <system/WindowBase.h>
#include <ui/AppState.h>

namespace WorkBalance::App {
namespace {
[[nodiscard]] int getWindowWidth() {
    return Core::Configuration::DEFAULT_WINDOW_WIDTH;
}

[[nodiscard]] int getWindowHeight() {
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    if (primary_monitor != nullptr) {
        const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
        if (mode != nullptr) {
            constexpr int taskbar_height = 150;
            return mode->height - taskbar_height;
        }
    }

    return Core::Configuration::DEFAULT_WINDOW_HEIGHT;
}

class ScopedGLFWContext {
  public:
    explicit ScopedGLFWContext(GLFWwindow* new_context) : m_previous(glfwGetCurrentContext()) {
        glfwMakeContextCurrent(new_context);
    }

    ScopedGLFWContext(const ScopedGLFWContext&) = delete;
    ScopedGLFWContext& operator=(const ScopedGLFWContext&) = delete;
    ScopedGLFWContext(ScopedGLFWContext&&) = delete;
    ScopedGLFWContext& operator=(ScopedGLFWContext&&) = delete;

    ~ScopedGLFWContext() {
        glfwMakeContextCurrent(m_previous);
    }

  private:
    GLFWwindow* m_previous{nullptr};
};
} // namespace

class Application::Impl {
  public:
    Impl();
    ~Impl();
    void run();

  private:
    void setupCallbacks();
    void updateTimer();
    void updateWellnessTimers();
    void handleTimerComplete();
    void handleWellnessTimerComplete(Core::WellnessType type);
    void updateOverlayState();
    void renderOverlayFrame();
    [[nodiscard]] bool shouldRenderOverlay() const noexcept;
    [[nodiscard]] bool isValidTaskIndex(size_t index) const noexcept;
    void adjustCurrentTaskIndex() noexcept;
    void updatePomodoroCounters();
    void updateWellnessCounters();
    void resetTimer();
    void setTimerMode(Core::TimerMode mode);
    void toggleTimer();
    void toggleOverlayMode();
    void requestClose();
    void applyDurations(int pomodoro_minutes, int short_break_minutes, int long_break_minutes);
    void applyWellnessSettings(int water_interval, int water_goal, int standup_interval, int standup_duration,
                               int eye_interval, int eye_break);
    void addTask(std::string_view name, int estimated);
    void removeTask(size_t index);
    void updateTask(size_t index, std::string_view name, int estimated, int completed);
    void toggleTaskCompletion(size_t index);
    void renderMainWindowFrame();
    void updateWindowTitle(int remaining_seconds);
    void loadPersistedData();
    void applyPersistedWindowPositions();
    void savePersistedData() const;
    void initializeSystemTray();
    void updateSystemTrayState();
    void showWindow();
    void setupWellnessCallbacks();

    // Wellness timer controls
    void toggleWaterTimer();
    void acknowledgeWater();
    void resetWaterDaily();
    void toggleStandupTimer();
    void acknowledgeStandup();
    void startStandupBreak();
    void endStandupBreak();
    void toggleEyeCareTimer();
    void acknowledgeEyeCare();
    void startEyeCareBreak();
    void endEyeCareBreak();

    template <typename Callback>
    void withAudio(Callback&& callback) {
        if (m_audio && m_audio->isInitialized()) {
            std::forward<Callback>(callback)(*m_audio);
        }
    }

    [[nodiscard]] static constexpr int minutesToSeconds(int minutes) noexcept {
        constexpr int seconds_per_minute = 60;
        return minutes * seconds_per_minute;
    }

  private:
    System::GLFWManager m_glfw_manager{};
    System::MainWindow m_window;
    App::ImGuiLayer m_imgui_layer;
    System::OverlayWindow m_overlay_window;
    std::unique_ptr<System::IAudioService> m_audio;
    Core::Timer m_timer;
    Core::TaskManager m_task_manager;
    Core::PersistenceManager m_persistence;
    System::SystemTray m_system_tray;
    AppState m_state;
    UI::MainWindowView m_main_view;
    UI::OverlayView m_overlay_view;

    // Wellness timers
    std::unique_ptr<Core::WellnessTimer> m_water_timer;
    std::unique_ptr<Core::WellnessTimer> m_standup_timer;
    std::unique_ptr<Core::WellnessTimer> m_eye_care_timer;
};

Application::Impl::Impl()
    : m_window(getWindowWidth(), getWindowHeight(), Core::Configuration::WINDOW_TITLE), m_imgui_layer(m_window.get()),
      m_overlay_window(), m_audio(System::createAudioService()),
      m_timer(Core::Configuration::DEFAULT_POMODORO_DURATION, Core::Configuration::DEFAULT_SHORT_BREAK_DURATION,
              Core::Configuration::DEFAULT_LONG_BREAK_DURATION),
      m_persistence(),
      m_main_view(
          m_window, m_imgui_layer, m_timer, m_task_manager, m_state,
          UI::MainWindowCallbacks{
              .onToggleTimer = [this]() { toggleTimer(); },
              .onModeChange = [this](Core::TimerMode mode) { setTimerMode(mode); },
              .onToggleOverlayMode = [this]() { toggleOverlayMode(); },
              .onRequestClose = [this]() { requestClose(); },
              .onDurationsApplied = [this](int pomodoro, int short_break,
                                           int long_break) { applyDurations(pomodoro, short_break, long_break); },
              .onWellnessSettingsApplied =
                  [this](int water_interval, int water_goal, int standup_interval, int standup_duration,
                         int eye_interval, int eye_break) {
                      applyWellnessSettings(water_interval, water_goal, standup_interval, standup_duration,
                                            eye_interval, eye_break);
                  },
              .onTaskAdded = [this](std::string_view name, int estimated) { addTask(name, estimated); },
              .onTaskRemoved = [this](size_t index) { removeTask(index); },
              .onTaskUpdated = [this](size_t index, std::string_view name, int estimated,
                                      int completed) { updateTask(index, name, estimated, completed); },
              .onTaskCompletionToggled = [this](size_t index) { toggleTaskCompletion(index); },
              .onTabChanged = [this](WorkBalance::NavigationTab /*tab*/) { /* Background color handled in view */ }}),
      m_overlay_view(m_imgui_layer, m_timer, m_state),
      m_water_timer(std::make_unique<Core::WellnessTimer>(Core::WellnessType::Water,
                                                          Core::WellnessDefaults::DEFAULT_WATER_INTERVAL)),
      m_standup_timer(std::make_unique<Core::WellnessTimer>(Core::WellnessType::Standup,
                                                            Core::WellnessDefaults::DEFAULT_STANDUP_INTERVAL,
                                                            Core::WellnessDefaults::DEFAULT_STANDUP_DURATION)),
      m_eye_care_timer(std::make_unique<Core::WellnessTimer>(Core::WellnessType::EyeStrain,
                                                             Core::WellnessDefaults::DEFAULT_EYE_INTERVAL,
                                                             Core::WellnessDefaults::DEFAULT_EYE_BREAK_DURATION)) {

    // Set up wellness timers in the views
    m_main_view.setWellnessTimers(m_water_timer.get(), m_standup_timer.get(), m_eye_care_timer.get());
    m_overlay_view.setWellnessTimers(m_water_timer.get(), m_standup_timer.get(), m_eye_care_timer.get());
    setupWellnessCallbacks();

    loadPersistedData();
    applyPersistedWindowPositions();
    m_state.background_color = WorkBalance::ThemeManager::getBackgroundColor(m_timer.getCurrentMode());
    updatePomodoroCounters();
    updateWellnessCounters();
    updateWindowTitle(m_timer.getRemainingTime());
    setupCallbacks();
    initializeSystemTray();
}

Application::Impl::~Impl() {
    savePersistedData();
}

void Application::Impl::run() {
    using clock = std::chrono::steady_clock;
    auto last_frame_time = clock::now();
    const std::chrono::duration<double> frame_duration{Core::Configuration::FRAME_TIME};

    while (!m_window.shouldClose()) {
        const auto current_time = clock::now();
        const std::chrono::duration<double> elapsed = current_time - last_frame_time;

        if (elapsed < frame_duration) {
            std::this_thread::sleep_for(frame_duration - elapsed);
        }

        last_frame_time = current_time;

        glfwPollEvents();
        m_system_tray.processMessages();
        updateTimer();
        updateWellnessTimers();
        updateSystemTrayState();

        m_imgui_layer.newFrame();
        m_main_view.render();
        renderMainWindowFrame();
        updateOverlayState();
        renderOverlayFrame();
    }
}

void Application::Impl::updateOverlayState() {
    if (m_state.show_timer_overlay == m_overlay_window.isVisible()) {
        return;
    }

    if (m_state.show_timer_overlay) {
        m_overlay_window.show();
    } else {
        m_overlay_window.hide();
    }
}

void Application::Impl::renderOverlayFrame() {
    if (!shouldRenderOverlay()) {
        return;
    }

    ScopedGLFWContext overlay_context(m_overlay_window.get());

    m_imgui_layer.newFrame();

    m_overlay_view.renderContent(m_overlay_window);
    m_overlay_view.renderFrame(m_overlay_window);
    m_overlay_window.swapBuffers();
}

bool Application::Impl::shouldRenderOverlay() const noexcept {
    return m_state.show_timer_overlay && m_overlay_window.get() != nullptr && !m_overlay_window.shouldClose();
}

void Application::Impl::setupCallbacks() {
    glfwSetWindowUserPointer(m_window.get(), this);
    glfwSetKeyCallback(m_window.get(), [](GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
        if (action != GLFW_PRESS) {
            return;
        }

        auto* app = static_cast<Application::Impl*>(glfwGetWindowUserPointer(window));
        if (app == nullptr) {
            return;
        }

        if (key == GLFW_KEY_UP) {
            app->toggleOverlayMode();
        } else if (key == GLFW_KEY_SPACE && !ImGui::GetIO().WantTextInput) {
            app->toggleTimer();
        } else if (key == GLFW_KEY_ESCAPE && app->m_state.main_window_overlay_mode) {
            app->toggleOverlayMode();
        }
    });
}

void Application::Impl::updateTimer() {
    const int previous_remaining = m_timer.getRemainingTime();
    [[maybe_unused]] const auto update_result = m_timer.update();

    const int current_remaining = m_timer.getRemainingTime();
    if (current_remaining != previous_remaining) {
        updateWindowTitle(current_remaining);
    }

    if (current_remaining <= 0 && m_timer.isRunning()) {
        handleTimerComplete();
    }
}

void Application::Impl::handleTimerComplete() {
    m_timer.stop();

    withAudio([](System::IAudioService& audio) { audio.playBellSound(); });

    if (m_timer.getCurrentMode() == Core::TimerMode::Pomodoro) {
        if (m_state.current_task_index >= 0 && isValidTaskIndex(static_cast<size_t>(m_state.current_task_index))) {
            m_task_manager.incrementTaskPomodoros(m_state.current_task_index);
        }

        updatePomodoroCounters();
    }

    resetTimer();
}

bool Application::Impl::isValidTaskIndex(size_t index) const noexcept {
    return index < m_task_manager.getTasks().size();
}

void Application::Impl::adjustCurrentTaskIndex() noexcept {
    const auto tasks = m_task_manager.getTasks();
    if (tasks.empty()) {
        m_state.current_task_index = 0;
        return;
    }

    m_state.current_task_index = std::clamp(m_state.current_task_index, 0, static_cast<int>(tasks.size()) - 1);
}

void Application::Impl::updatePomodoroCounters() {
    m_state.target_pomodoros = m_task_manager.getTargetPomodoros();
    m_state.completed_pomodoros = m_task_manager.getCompletedPomodoros();
}

void Application::Impl::resetTimer() {
    m_timer.reset();
    updateWindowTitle(m_timer.getRemainingTime());
}

void Application::Impl::setTimerMode(Core::TimerMode mode) {
    m_timer.setMode(mode);
    m_state.background_color = WorkBalance::ThemeManager::getBackgroundColor(mode);
    resetTimer();
}

void Application::Impl::toggleTimer() {
    withAudio([](System::IAudioService& audio) { audio.playClickSound(); });
    m_timer.toggle();
}

void Application::Impl::toggleOverlayMode() {
    m_state.main_window_overlay_mode = !m_state.main_window_overlay_mode;
    m_window.setOverlayMode(m_state.main_window_overlay_mode);
}

void Application::Impl::requestClose() {
    glfwSetWindowShouldClose(m_window.get(), GLFW_TRUE);
}

void Application::Impl::applyDurations(int pomodoro_minutes, int short_break_minutes, int long_break_minutes) {
    m_timer.setPomodoroDuration(minutesToSeconds(pomodoro_minutes));
    m_timer.setShortBreakDuration(minutesToSeconds(short_break_minutes));
    m_timer.setLongBreakDuration(minutesToSeconds(long_break_minutes));

    if (m_timer.getState() == Core::TimerState::Stopped) {
        resetTimer();
    }
}

void Application::Impl::applyWellnessSettings(int water_interval, int water_goal, int standup_interval,
                                              int standup_duration, int eye_interval, int eye_break) {
    if (m_water_timer) {
        m_water_timer->setIntervalSeconds(water_interval * 60);
    }
    m_state.water_daily_goal = water_goal;

    if (m_standup_timer) {
        m_standup_timer->setIntervalSeconds(standup_interval * 60);
        m_standup_timer->setBreakDurationSeconds(standup_duration * 60);
    }

    if (m_eye_care_timer) {
        m_eye_care_timer->setIntervalSeconds(eye_interval * 60);
        m_eye_care_timer->setBreakDurationSeconds(eye_break);
    }
}

void Application::Impl::addTask(std::string_view name, int estimated) {
    m_task_manager.addTask(name, estimated);
    updatePomodoroCounters();
}

void Application::Impl::removeTask(size_t index) {
    if (!isValidTaskIndex(index)) {
        return;
    }

    m_task_manager.removeTask(index);
    adjustCurrentTaskIndex();
    updatePomodoroCounters();
}

void Application::Impl::updateTask(size_t index, std::string_view name, int estimated, int completed) {
    if (!isValidTaskIndex(index)) {
        return;
    }

    m_task_manager.updateTask(index, name, estimated, completed);
    updatePomodoroCounters();
}

void Application::Impl::toggleTaskCompletion(size_t index) {
    if (!isValidTaskIndex(index)) {
        return;
    }

    m_task_manager.toggleTaskCompletion(index);
    updatePomodoroCounters();
}

void Application::Impl::renderMainWindowFrame() {
    const auto [width, height] = m_window.getFramebufferSize();
    glViewport(0, 0, width, height);

    if (m_state.main_window_overlay_mode) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        const auto& color = m_state.background_color;
        glClearColor(color.x, color.y, color.z, color.w);
    }

    glClear(GL_COLOR_BUFFER_BIT);
    m_imgui_layer.render();

    if (m_state.main_window_overlay_mode) {
        glDisable(GL_BLEND);
    }

    m_window.swapBuffers();
}

void Application::Impl::updateWindowTitle(int remaining_seconds) {
    std::string time_str = WorkBalance::TimeFormatter::formatTime(remaining_seconds);
    std::string title = "Work Balance - " + time_str;
    glfwSetWindowTitle(m_window.get(), title.c_str());

    // Update system tray tooltip
    m_system_tray.setTooltip("WorkBalance - " + time_str);
}

void Application::Impl::loadPersistedData() {
    auto loaded_data = m_persistence.load();
    if (!loaded_data.has_value()) {
        return;
    }

    const auto& data = loaded_data.value();

    // Apply saved timer durations
    m_timer.setPomodoroDuration(minutesToSeconds(data.settings.pomodoro_duration_minutes));
    m_timer.setShortBreakDuration(minutesToSeconds(data.settings.short_break_duration_minutes));
    m_timer.setLongBreakDuration(minutesToSeconds(data.settings.long_break_duration_minutes));
    m_timer.reset();

    // Update settings in UI state
    m_state.temp_pomodoro_duration = data.settings.pomodoro_duration_minutes;
    m_state.temp_short_break_duration = data.settings.short_break_duration_minutes;
    m_state.temp_long_break_duration = data.settings.long_break_duration_minutes;

    // Restore overlay position
    m_state.overlay_position = ImVec2(data.settings.overlay_position_x, data.settings.overlay_position_y);

    // Restore main window position
    m_state.main_window_x = data.settings.main_window_x;
    m_state.main_window_y = data.settings.main_window_y;

    // Restore overlay visibility settings
    m_state.show_pomodoro_in_overlay = data.settings.show_pomodoro_in_overlay;
    m_state.show_water_in_overlay = data.settings.show_water_in_overlay;
    m_state.show_standup_in_overlay = data.settings.show_standup_in_overlay;
    m_state.show_eye_care_in_overlay = data.settings.show_eye_care_in_overlay;

    // Restore tasks
    for (const auto& task : data.tasks) {
        m_task_manager.addTask(task.name, task.estimated_pomodoros);
        auto* added_task = m_task_manager.getTask(m_task_manager.getTaskCount() - 1);
        if (added_task != nullptr) {
            added_task->completed = task.completed;
            added_task->completed_pomodoros = task.completed_pomodoros;
        }
    }

    // Restore current task index
    m_state.current_task_index = data.current_task_index;
    adjustCurrentTaskIndex();
}

void Application::Impl::applyPersistedWindowPositions() {
    // Apply main window position if it was saved (not default -1)
    if (m_state.main_window_x >= 0 && m_state.main_window_y >= 0) {
        m_window.setPosition(m_state.main_window_x, m_state.main_window_y);
    }

    // Apply overlay window position
    m_overlay_window.setPosition(static_cast<int>(m_state.overlay_position.x),
                                 static_cast<int>(m_state.overlay_position.y));
}

void Application::Impl::savePersistedData() const {
    Core::PersistentData data;

    // Save timer durations (convert seconds to minutes)
    constexpr int seconds_per_minute = 60;
    data.settings.pomodoro_duration_minutes = m_timer.getPomodoroDuration() / seconds_per_minute;
    data.settings.short_break_duration_minutes = m_timer.getShortBreakDuration() / seconds_per_minute;
    data.settings.long_break_duration_minutes = m_timer.getLongBreakDuration() / seconds_per_minute;

    // Save overlay position
    data.settings.overlay_position_x = m_state.overlay_position.x;
    data.settings.overlay_position_y = m_state.overlay_position.y;

    // Save main window position (get current position from window)
    const auto [win_x, win_y] = m_window.getPosition();
    data.settings.main_window_x = win_x;
    data.settings.main_window_y = win_y;

    // Save overlay visibility settings
    data.settings.show_pomodoro_in_overlay = m_state.show_pomodoro_in_overlay;
    data.settings.show_water_in_overlay = m_state.show_water_in_overlay;
    data.settings.show_standup_in_overlay = m_state.show_standup_in_overlay;
    data.settings.show_eye_care_in_overlay = m_state.show_eye_care_in_overlay;

    // Save tasks
    const auto tasks = m_task_manager.getTasks();
    data.tasks.reserve(tasks.size());
    for (const auto& task : tasks) {
        data.tasks.push_back(task);
    }

    // Save current task index
    data.current_task_index = m_state.current_task_index;

    [[maybe_unused]] const auto save_result = m_persistence.save(data);
}

void Application::Impl::initializeSystemTray() {
    [[maybe_unused]] const bool initialized =
        m_system_tray.initialize(System::SystemTrayCallbacks{.onToggleTimer = [this]() { toggleTimer(); },
                                                             .onToggleOverlayMode = [this]() { toggleOverlayMode(); },
                                                             .onShowWindow = [this]() { showWindow(); },
                                                             .onQuit = [this]() { requestClose(); }});

    // Set initial state
    m_system_tray.updateTimerState(m_timer.isRunning());
    m_system_tray.updateWindowMode(m_state.main_window_overlay_mode);
    m_system_tray.setTooltip("WorkBalance - " + WorkBalance::TimeFormatter::formatTime(m_timer.getRemainingTime()));
}

void Application::Impl::updateSystemTrayState() {
    m_system_tray.updateTimerState(m_timer.isRunning());
    m_system_tray.updateWindowMode(m_state.main_window_overlay_mode);
}

void Application::Impl::showWindow() {
    glfwShowWindow(m_window.get());
    glfwFocusWindow(m_window.get());
}

// ============================================================================
// Wellness Timer Methods
// ============================================================================

void Application::Impl::setupWellnessCallbacks() {
    m_main_view.setWellnessCallbacks(UI::WellnessCallbacks{.onWaterToggle = [this]() { toggleWaterTimer(); },
                                                           .onWaterAcknowledge = [this]() { acknowledgeWater(); },
                                                           .onWaterResetDaily = [this]() { resetWaterDaily(); },

                                                           .onStandupToggle = [this]() { toggleStandupTimer(); },
                                                           .onStandupAcknowledge = [this]() { acknowledgeStandup(); },
                                                           .onStandupStartBreak = [this]() { startStandupBreak(); },
                                                           .onStandupEndBreak = [this]() { endStandupBreak(); },

                                                           .onEyeCareToggle = [this]() { toggleEyeCareTimer(); },
                                                           .onEyeCareAcknowledge = [this]() { acknowledgeEyeCare(); },
                                                           .onEyeCareStartBreak = [this]() { startEyeCareBreak(); },
                                                           .onEyeCareEndBreak = [this]() { endEyeCareBreak(); }});
}

void Application::Impl::updateWellnessTimers() {
    if (m_water_timer && m_water_timer->update()) {
        handleWellnessTimerComplete(Core::WellnessType::Water);
    }
    if (m_standup_timer && m_standup_timer->update()) {
        handleWellnessTimerComplete(Core::WellnessType::Standup);
    }
    if (m_eye_care_timer && m_eye_care_timer->update()) {
        handleWellnessTimerComplete(Core::WellnessType::EyeStrain);
    }

    updateWellnessCounters();
}

void Application::Impl::handleWellnessTimerComplete(Core::WellnessType type) {
    withAudio([](System::IAudioService& audio) { audio.playBellSound(); });

    // For timers with breaks, the completion is handled differently
    switch (type) {
        case Core::WellnessType::Water:
            // Water reminder - just notify user
            break;
        case Core::WellnessType::Standup:
            // If break completed, restart interval timer
            if (m_standup_timer && !m_standup_timer->isInBreak()) {
                m_standup_timer->start();
            }
            break;
        case Core::WellnessType::EyeStrain:
            // If break completed, restart interval timer
            if (m_eye_care_timer && !m_eye_care_timer->isInBreak()) {
                m_eye_care_timer->start();
            }
            break;
        default:
            break;
    }
}

void Application::Impl::updateWellnessCounters() {
    if (m_water_timer) {
        m_state.water_glasses_consumed = m_water_timer->getCompletedCount();
    }
    if (m_standup_timer) {
        m_state.standups_completed = m_standup_timer->getCompletedCount();
    }
    if (m_eye_care_timer) {
        m_state.eye_breaks_completed = m_eye_care_timer->getCompletedCount();
    }
}

void Application::Impl::toggleWaterTimer() {
    if (m_water_timer) {
        withAudio([](System::IAudioService& audio) { audio.playClickSound(); });
        m_water_timer->toggle();
    }
}

void Application::Impl::acknowledgeWater() {
    if (m_water_timer) {
        withAudio([](System::IAudioService& audio) { audio.playClickSound(); });
        m_water_timer->acknowledgeReminder();
        updateWellnessCounters();
    }
}

void Application::Impl::resetWaterDaily() {
    if (m_water_timer) {
        m_water_timer->resetDailyCounters();
        m_water_timer->reset();
        updateWellnessCounters();
    }
}

void Application::Impl::toggleStandupTimer() {
    if (m_standup_timer) {
        withAudio([](System::IAudioService& audio) { audio.playClickSound(); });
        m_standup_timer->toggle();
    }
}

void Application::Impl::acknowledgeStandup() {
    if (m_standup_timer) {
        // Skip this reminder
        m_standup_timer->acknowledgeReminder();
        m_standup_timer->reset();
        m_standup_timer->start();
    }
}

void Application::Impl::startStandupBreak() {
    if (m_standup_timer) {
        withAudio([](System::IAudioService& audio) { audio.playClickSound(); });
        m_standup_timer->startBreak();
    }
}

void Application::Impl::endStandupBreak() {
    if (m_standup_timer) {
        withAudio([](System::IAudioService& audio) { audio.playClickSound(); });
        m_standup_timer->endBreak();
        updateWellnessCounters();
    }
}

void Application::Impl::toggleEyeCareTimer() {
    if (m_eye_care_timer) {
        withAudio([](System::IAudioService& audio) { audio.playClickSound(); });
        m_eye_care_timer->toggle();
    }
}

void Application::Impl::acknowledgeEyeCare() {
    if (m_eye_care_timer) {
        // Skip this reminder
        m_eye_care_timer->acknowledgeReminder();
        m_eye_care_timer->reset();
        m_eye_care_timer->start();
    }
}

void Application::Impl::startEyeCareBreak() {
    if (m_eye_care_timer) {
        withAudio([](System::IAudioService& audio) { audio.playClickSound(); });
        m_eye_care_timer->startBreak();
    }
}

void Application::Impl::endEyeCareBreak() {
    if (m_eye_care_timer) {
        withAudio([](System::IAudioService& audio) { audio.playClickSound(); });
        m_eye_care_timer->endBreak();
        updateWellnessCounters();
    }
}

Application::Application() : m_impl(std::make_unique<Application::Impl>()) {
}

Application::~Application() = default;

void Application::run() {
    m_impl->run();
}

} // namespace WorkBalance::App
