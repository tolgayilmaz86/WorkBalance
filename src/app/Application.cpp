#define NOMINMAX
#include <app/Application.h>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <thread>

#include <app/ImGuiLayer.h>
#include <app/ui/MainWindowView.h>
#include <app/ui/OverlayView.h>
#include <core/Configuration.h>
#include <core/Task.h>
#include <core/Timer.h>
#include <system/AudioManager.h>
#include <system/MainWindow.h>
#include <system/OverlayWindow.h>
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
} // namespace

class Application::Impl {
  public:
    Impl();
    void run();

  private:
    void setupCallbacks();
    void initializeTasks();
    void updateTimer();
    void handleTimerComplete();
    void updatePomodoroCounters();
    void resetTimer();
    void setTimerMode(Core::TimerMode mode);
    void toggleTimer();
    void toggleOverlayMode();
    void requestClose();
    void applyDurations(int pomodoro_minutes, int short_break_minutes, int long_break_minutes);
    void addTask(std::string_view name, int estimated);
    void removeTask(size_t index);
    void updateTask(size_t index, std::string_view name, int estimated, int completed);
    void toggleTaskCompletion(size_t index);
    void renderMainWindowFrame();
    void updateWindowTitle(int remaining_seconds);

  private:
    System::GLFWManager m_glfw_manager{};
    System::MainWindow m_window;
    App::ImGuiLayer m_imgui_layer;
    System::OverlayWindow m_overlay_window;
    std::unique_ptr<System::IAudioService> m_audio;
    Core::Timer m_timer;
    Core::TaskManager m_task_manager;
    AppState m_state;
    UI::MainWindowView m_main_view;
    UI::OverlayView m_overlay_view;
};

Application::Impl::Impl()
    : m_window(getWindowWidth(), getWindowHeight(), Core::Configuration::WINDOW_TITLE), m_imgui_layer(m_window.get()),
      m_overlay_window(), m_audio(System::createAudioService()),
      m_timer(Core::Configuration::DEFAULT_POMODORO_DURATION, Core::Configuration::DEFAULT_SHORT_BREAK_DURATION,
              Core::Configuration::DEFAULT_LONG_BREAK_DURATION),
      m_main_view(
          m_window, m_imgui_layer, m_timer, m_task_manager, m_state,
          UI::MainWindowCallbacks{
              .onToggleTimer = [this]() { toggleTimer(); },
              .onModeChange = [this](Core::TimerMode mode) { setTimerMode(mode); },
              .onToggleOverlayMode = [this]() { toggleOverlayMode(); },
              .onRequestClose = [this]() { requestClose(); },
              .onDurationsApplied = [this](int pomodoro, int short_break,
                                           int long_break) { applyDurations(pomodoro, short_break, long_break); },
              .onTaskAdded = [this](std::string_view name, int estimated) { addTask(name, estimated); },
              .onTaskRemoved = [this](size_t index) { removeTask(index); },
              .onTaskUpdated = [this](size_t index, std::string_view name, int estimated,
                                      int completed) { updateTask(index, name, estimated, completed); },
              .onTaskCompletionToggled = [this](size_t index) { toggleTaskCompletion(index); }}),
      m_overlay_view(m_imgui_layer, m_timer, m_state) {
    m_state.background_color = WorkBalance::ThemeManager::getBackgroundColor(m_timer.getCurrentMode());
    initializeTasks();
    updateWindowTitle(m_timer.getRemainingTime());
    setupCallbacks();
}

void Application::Impl::run() {
    auto last_frame_time = std::chrono::high_resolution_clock::now();

    while (!m_window.shouldClose()) {
        const auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = current_time - last_frame_time;

        if (elapsed.count() < Core::Configuration::FRAME_TIME) {
            std::this_thread::sleep_for(
                std::chrono::duration<double>(Core::Configuration::FRAME_TIME - elapsed.count()));
        }

        last_frame_time = current_time;

        glfwPollEvents();
        updateTimer();

        m_imgui_layer.newFrame();
        m_main_view.render();
        renderMainWindowFrame();

        if (m_state.show_timer_overlay && !m_overlay_window.isVisible()) {
            m_overlay_window.show();
        } else if (!m_state.show_timer_overlay && m_overlay_window.isVisible()) {
            m_overlay_window.hide();
        }

        GLFWwindow* overlay_handle = m_overlay_window.get();
        if (m_state.show_timer_overlay && overlay_handle != nullptr && !m_overlay_window.shouldClose()) {
            glfwMakeContextCurrent(overlay_handle);
            m_imgui_layer.newFrame();
            m_overlay_view.renderContent(m_overlay_window);
            m_overlay_view.renderFrame(m_overlay_window);
            m_overlay_window.swapBuffers();
            glfwMakeContextCurrent(m_window.get());
        }
    }
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

void Application::Impl::initializeTasks() {
    updatePomodoroCounters();
}

void Application::Impl::updateTimer() {
    const int previous_remaining = m_timer.getRemainingTime();
    m_timer.update();

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

    if (m_audio && m_audio->isInitialized()) {
        m_audio->playBellSound();
    }

    if (m_timer.getCurrentMode() == Core::TimerMode::Pomodoro) {
        if (!m_task_manager.getTasks().empty() &&
            m_state.current_task_index < static_cast<int>(m_task_manager.getTasks().size())) {
            m_task_manager.incrementTaskPomodoros(m_state.current_task_index);
        }

        updatePomodoroCounters();
    }

    resetTimer();
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
    if (m_audio && m_audio->isInitialized()) {
        m_audio->playClickSound();
    }
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
    m_timer.setPomodoroDuration(pomodoro_minutes * 60);
    m_timer.setShortBreakDuration(short_break_minutes * 60);
    m_timer.setLongBreakDuration(long_break_minutes * 60);

    if (m_timer.getState() == Core::TimerState::Stopped) {
        resetTimer();
    }
}

void Application::Impl::addTask(std::string_view name, int estimated) {
    m_task_manager.addTask(name, estimated);
    updatePomodoroCounters();
}

void Application::Impl::removeTask(size_t index) {
    if (index >= m_task_manager.getTasks().size()) {
        return;
    }

    m_task_manager.removeTask(index);

    if (m_task_manager.getTasks().empty()) {
        m_state.current_task_index = 0;
    } else if (m_state.current_task_index >= static_cast<int>(m_task_manager.getTasks().size())) {
        m_state.current_task_index = static_cast<int>(m_task_manager.getTasks().size()) - 1;
    }

    updatePomodoroCounters();
}

void Application::Impl::updateTask(size_t index, std::string_view name, int estimated, int completed) {
    if (index >= m_task_manager.getTasks().size()) {
        return;
    }

    m_task_manager.updateTask(index, name, estimated, completed);
    updatePomodoroCounters();
}

void Application::Impl::toggleTaskCompletion(size_t index) {
    if (index >= m_task_manager.getTasks().size()) {
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
    std::string title = "Work Balance - " + WorkBalance::TimeFormatter::formatTime(remaining_seconds);
    glfwSetWindowTitle(m_window.get(), title.c_str());
}

Application::Application() : m_impl(std::make_unique<Application::Impl>()) {
}

Application::~Application() = default;

Application::Application(Application&&) noexcept = default;
Application& Application::operator=(Application&&) noexcept = default;

void Application::run() {
    m_impl->run();
}

} // namespace WorkBalance::App
