#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <app/ui/MainWindowView.h>
#include <app/ui/WellnessViews.h>

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>

#include "assets/fonts/IconsFontAwesome5Pro.h"
#include <core/Configuration.h>
#include <core/WellnessTypes.h>
#include <ui/AppState.h>

namespace WorkBalance::App::UI {
namespace {
struct WindowCoordinates {
    int window_x = 0;
    int window_y = 0;
    double cursor_x = 0.0;
    double cursor_y = 0.0;
};

[[nodiscard]] WindowCoordinates queryWindowCoordinates(GLFWwindow* handle) {
    WindowCoordinates coordinates{};
    if (handle == nullptr) {
        return coordinates;
    }

    glfwGetWindowPos(handle, &coordinates.window_x, &coordinates.window_y);
    glfwGetCursorPos(handle, &coordinates.cursor_x, &coordinates.cursor_y);
    coordinates.cursor_x += static_cast<double>(coordinates.window_x);
    coordinates.cursor_y += static_cast<double>(coordinates.window_y);
    return coordinates;
}

class ScopedFont {
  public:
    explicit ScopedFont(ImFont* font) : m_font(font) {
        if (m_font != nullptr) {
            ImGui::PushFont(m_font);
        }
    }

    ScopedFont(const ScopedFont&) = delete;
    ScopedFont& operator=(const ScopedFont&) = delete;
    ScopedFont(ScopedFont&&) = delete;
    ScopedFont& operator=(ScopedFont&&) = delete;

    ~ScopedFont() {
        if (m_font != nullptr) {
            ImGui::PopFont();
        }
    }

  private:
    ImFont* m_font;
};

[[nodiscard]] ImVec2 calculateTextSize(ImFont* font, std::string_view text) {
    const char* const start = text.data();
    const char* const end = start + text.size();

    if (font == nullptr) {
        return ImGui::CalcTextSize(start, end);
    }

    ScopedFont scoped_font(font);
    return ImGui::CalcTextSize(start, end);
}

void updateWindowDragging(GLFWwindow* handle, bool hovered, bool& dragging, ImVec2& offset,
                          float drag_threshold = 0.0f) {
    if (handle == nullptr) {
        dragging = false;
        return;
    }

    if (hovered && ImGui::IsMouseClicked(0)) {
        const WindowCoordinates coordinates = queryWindowCoordinates(handle);
        offset = ImVec2(static_cast<float>(coordinates.cursor_x - coordinates.window_x),
                        static_cast<float>(coordinates.cursor_y - coordinates.window_y));
        dragging = true;
    }

    if (!dragging) {
        return;
    }

    if (ImGui::IsMouseDragging(0, drag_threshold)) {
        const WindowCoordinates coordinates = queryWindowCoordinates(handle);
        const int new_x = static_cast<int>(coordinates.cursor_x - offset.x);
        const int new_y = static_cast<int>(coordinates.cursor_y - offset.y);
        glfwSetWindowPos(handle, new_x, new_y);
    }

    if (ImGui::IsMouseReleased(0)) {
        dragging = false;
    }
}
} // namespace
MainWindowView::MainWindowView(System::MainWindow& window, App::ImGuiLayer& imgui, Core::Timer& timer,
                               Core::TaskManager& taskManager, AppState& state, MainWindowCallbacks callbacks)
    : m_window(window), m_imgui(imgui), m_timer(timer), m_task_manager(taskManager), m_state(state),
      m_callbacks(std::move(callbacks)) {

    // Initialize navigation tabs
    m_navigation_tabs = std::make_unique<WorkBalance::UI::NavigationTabs>(
        m_state, WorkBalance::UI::NavigationCallbacks{.onTabChanged = [this](NavigationTab tab) {
            updateBackgroundColor();
            if (m_callbacks.onTabChanged) {
                m_callbacks.onTabChanged(tab);
            }
        }});

    // Initialize settings popup component
    m_settings_popup = std::make_unique<Components::SettingsPopup>(
        m_state,
        Components::SettingsPopup::Callbacks{.onPomodoroDurationsApplied = m_callbacks.onDurationsApplied,
                                             .onWellnessSettingsApplied = m_callbacks.onWellnessSettingsApplied});

    // Initialize timer panel component
    m_timer_panel = std::make_unique<Components::TimerPanel>(
        m_timer, m_state, m_imgui,
        Components::TimerPanel::Callbacks{.onToggle = m_callbacks.onToggleTimer,
                                          .onModeChange = m_callbacks.onModeChange});

    // Initialize task list panel component
    m_task_list_panel = std::make_unique<Components::TaskListPanel>(
        m_task_manager, m_state,
        Components::TaskListPanel::Callbacks{.onTaskAdded = m_callbacks.onTaskAdded,
                                             .onTaskRemoved = m_callbacks.onTaskRemoved,
                                             .onTaskUpdated = m_callbacks.onTaskUpdated,
                                             .onTaskToggled = m_callbacks.onTaskCompletionToggled,
                                             .onTaskMoved = m_callbacks.onTaskMoved});
}

void MainWindowView::setWellnessTimers(Core::WellnessTimer* water, Core::WellnessTimer* standup,
                                       Core::WellnessTimer* eyeCare) {
    m_water_timer = water;
    m_standup_timer = standup;
    m_eye_care_timer = eyeCare;
}

void MainWindowView::setWellnessCallbacks(WellnessCallbacks callbacks) {
    m_wellness_callbacks = std::move(callbacks);
}

void MainWindowView::updateBackgroundColor() {
    switch (m_state.active_tab) {
        case NavigationTab::Pomodoro:
            m_state.background_color = WorkBalance::ThemeManager::getBackgroundColor(m_timer.getCurrentMode());
            break;
        case NavigationTab::Water:
            m_state.background_color = Core::WellnessDefaults::WATER_BG_COLOR;
            break;
        case NavigationTab::Standup:
            m_state.background_color = Core::WellnessDefaults::STANDUP_BG_COLOR;
            break;
        case NavigationTab::EyeCare:
            m_state.background_color = Core::WellnessDefaults::EYE_STRAIN_BG_COLOR;
            break;
    }
}

void MainWindowView::render() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Main window takes full viewport (tabs are now inside the window)
    const ImVec2 main_window_pos(viewport->WorkPos.x, viewport->WorkPos.y);
    const ImVec2 main_window_size(viewport->WorkSize.x, viewport->WorkSize.y);

    // Now render the main window
    ImGui::SetNextWindowPos(main_window_pos);
    ImGui::SetNextWindowSize(main_window_size);

    constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("WorkBalance", nullptr, window_flags)) {
        if (m_state.main_window_overlay_mode) {
            renderOverlayMode();
        } else {
            renderHeader();

            // Get header height for content positioning
            const float header_height = ImGui::GetCursorPosY();
            const ImVec2 window_size = ImGui::GetWindowSize();

            // Main content uses FULL window width now (tabs are outside)
            const float available_height = window_size.y - header_height;

            // Begin a child region for the main content (full width)
            ImGui::BeginChild("ContentRegion", ImVec2(0, available_height), false);

            // Render content based on active tab
            switch (m_state.active_tab) {
                case NavigationTab::Pomodoro:
                    renderPomodoroContent();
                    break;
                case NavigationTab::Water:
                    renderWaterContent();
                    break;
                case NavigationTab::Standup:
                    renderStandupContent();
                    break;
                case NavigationTab::EyeCare:
                    renderEyeCareContent();
                    break;
            }

            ImGui::EndChild();

            if (m_settings_popup) {
                m_settings_popup->render();
            }
            renderEditTaskPopup();
            renderHelpPopup();

            handleWindowDragging();
        }
    }
    ImGui::End();
}

void MainWindowView::renderNavigationTabs(const ImVec2& window_pos, const ImVec2& window_size, float header_height) {
    if (m_navigation_tabs) {
        m_navigation_tabs->render(window_pos, window_size, header_height);
    }
}

void MainWindowView::renderPomodoroContent() {
    // Render navigation tabs just above the timer frame
    if (m_navigation_tabs) {
        m_navigation_tabs->renderInline();
    }
    ImGui::Spacing();

    if (m_timer_panel) {
        m_timer_panel->render();
    }
    renderCurrentTask();
    if (m_task_list_panel) {
        m_task_list_panel->render();
    }
    renderPomodoroCounter();
}

void MainWindowView::renderWaterContent() {
    // Render navigation tabs just above the content
    if (m_navigation_tabs) {
        m_navigation_tabs->renderInline();
    }
    ImGui::Spacing();

    if (m_water_timer != nullptr) {
        WaterReminderView view(m_imgui, *m_water_timer, m_state,
                               WellnessViewCallbacks{.onToggleTimer = m_wellness_callbacks.onWaterToggle,
                                                     .onAcknowledge = m_wellness_callbacks.onWaterAcknowledge,
                                                     .onResetDaily = m_wellness_callbacks.onWaterResetDaily});
        view.render();
    } else {
        ImGui::TextUnformatted("Water reminder not initialized");
    }
}

void MainWindowView::renderStandupContent() {
    // Render navigation tabs just above the content
    if (m_navigation_tabs) {
        m_navigation_tabs->renderInline();
    }
    ImGui::Spacing();

    if (m_standup_timer != nullptr) {
        StandupReminderView view(m_imgui, *m_standup_timer, m_state,
                                 WellnessViewCallbacks{.onToggleTimer = m_wellness_callbacks.onStandupToggle,
                                                       .onAcknowledge = m_wellness_callbacks.onStandupAcknowledge,
                                                       .onStartBreak = m_wellness_callbacks.onStandupStartBreak,
                                                       .onEndBreak = m_wellness_callbacks.onStandupEndBreak});
        view.render();
    } else {
        ImGui::TextUnformatted("Standup reminder not initialized");
    }
}

void MainWindowView::renderEyeCareContent() {
    // Render navigation tabs just above the content
    if (m_navigation_tabs) {
        m_navigation_tabs->renderInline();
    }
    ImGui::Spacing();

    if (m_eye_care_timer != nullptr) {
        EyeCareReminderView view(m_imgui, *m_eye_care_timer, m_state,
                                 WellnessViewCallbacks{.onToggleTimer = m_wellness_callbacks.onEyeCareToggle,
                                                       .onAcknowledge = m_wellness_callbacks.onEyeCareAcknowledge,
                                                       .onStartBreak = m_wellness_callbacks.onEyeCareStartBreak,
                                                       .onEndBreak = m_wellness_callbacks.onEyeCareEndBreak});
        view.render();
    } else {
        ImGui::TextUnformatted("Eye care reminder not initialized");
    }
}

void MainWindowView::renderOverlayMode() {
    // Count active wellness timers that are visible in overlay to decide format
    int active_wellness_count = 0;
    if (m_water_timer != nullptr && m_water_timer->isRunning() && m_state.show_water_in_overlay) {
        active_wellness_count++;
    }
    if (m_standup_timer != nullptr && m_standup_timer->isRunning() && m_state.show_standup_in_overlay) {
        active_wellness_count++;
    }
    if (m_eye_care_timer != nullptr && m_eye_care_timer->isRunning() && m_state.show_eye_care_in_overlay) {
        active_wellness_count++;
    }

    // Build horizontal compact display: 🕐 25:00 | 💧 45m | 🚶 30m | 👁 20m
    // Use compact format for all timers when multiple are active
    std::string display_str;
    if (m_state.show_pomodoro_in_overlay) {
        if (active_wellness_count > 0) {
            // Multiple timers - use compact format for all (no seconds)
            display_str = WorkBalance::TimeFormatter::formatTimerWithIconCompact(m_timer.getCurrentMode(),
                                                                                 m_timer.getRemainingTime());
        } else {
            // Single timer - show full format with seconds
            display_str =
                WorkBalance::TimeFormatter::formatTimerWithIcon(m_timer.getCurrentMode(), m_timer.getRemainingTime());
        }
    }

    // Add wellness timers if they exist, are running, and enabled in overlay
    if (m_water_timer != nullptr && m_water_timer->isRunning() && m_state.show_water_in_overlay) {
        if (!display_str.empty()) {
            display_str += "  |  ";
        }
        display_str += WorkBalance::TimeFormatter::getWellnessIcon(Core::WellnessType::Water);
        display_str += " ";
        display_str += WorkBalance::TimeFormatter::formatTimeCompact(m_water_timer->getRemainingTime());
    }
    if (m_standup_timer != nullptr && m_standup_timer->isRunning() && m_state.show_standup_in_overlay) {
        if (!display_str.empty()) {
            display_str += "  |  ";
        }
        display_str += WorkBalance::TimeFormatter::getWellnessIcon(Core::WellnessType::Standup);
        display_str += " ";
        display_str += WorkBalance::TimeFormatter::formatTimeCompact(m_standup_timer->getRemainingTime());
    }
    if (m_eye_care_timer != nullptr && m_eye_care_timer->isRunning() && m_state.show_eye_care_in_overlay) {
        if (!display_str.empty()) {
            display_str += "  |  ";
        }
        display_str += WorkBalance::TimeFormatter::getWellnessIcon(Core::WellnessType::EyeStrain);
        display_str += " ";
        display_str += WorkBalance::TimeFormatter::formatTimeCompact(m_eye_care_timer->getRemainingTime());
    }

    ImFont* overlay_font = m_imgui.overlayFont();
    const float font_scale = (active_wellness_count > 0) ? 0.7f : 1.0f;

    // Calculate text size with font scale
    ImGui::PushFont(overlay_font);
    ImGui::SetWindowFontScale(font_scale);
    const ImVec2 text_size = ImGui::CalcTextSize(display_str.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();

    // Dynamically resize window based on content
    constexpr float padding_x = 40.0f;
    constexpr float padding_y = 20.0f;
    const int required_width = static_cast<int>(text_size.x + padding_x);
    const int required_height = static_cast<int>(text_size.y + padding_y);

    int current_width = 0;
    int current_height = 0;
    glfwGetWindowSize(m_window.get(), &current_width, &current_height);
    if (current_width != required_width || current_height != required_height) {
        glfwSetWindowSize(m_window.get(), required_width, required_height);
    }

    const float window_width = ImGui::GetWindowSize().x;
    const float window_height = ImGui::GetWindowSize().y;

    ImGui::SetCursorPos(ImVec2((window_width - text_size.x) * 0.5f, (window_height - text_size.y) * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    {
        ScopedFont font_scope(overlay_font);
        ImGui::SetWindowFontScale(font_scale);
        ImGui::TextUnformatted(display_str.c_str());
        ImGui::SetWindowFontScale(1.0f);
    }
    ImGui::PopStyleColor();

    const bool overlay_hovered = ImGui::IsWindowHovered();

    // Handle double-click to exit overlay mode (same as pressing ESC)
    if (overlay_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (m_callbacks.onToggleOverlayMode) {
            m_callbacks.onToggleOverlayMode();
        }
    }

    updateWindowDragging(m_window.get(), overlay_hovered, m_state.main_overlay_dragging,
                         m_state.main_overlay_drag_offset);
}

void MainWindowView::renderHeader() {
    const float window_width = ImGui::GetWindowSize().x;
    constexpr float button_size = 32.0f;
    constexpr float spacing = 16.0f;

    const float total_width = (button_size * 4.0f) + (spacing * 3.0f);
    const float start_x = (window_width - total_width) * 0.5f;

    ImGui::SetCursorPosX(start_x);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.8f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    if (ImGui::Button(ICON_FA_COG, ImVec2(button_size, button_size))) {
        m_state.show_settings = true;
        // Pomodoro settings
        m_state.temp_pomodoro_duration = m_timer.getPomodoroDuration() / 60;
        m_state.temp_short_break_duration = m_timer.getShortBreakDuration() / 60;
        m_state.temp_long_break_duration = m_timer.getLongBreakDuration() / 60;
        // Wellness settings
        if (m_water_timer != nullptr) {
            m_state.temp_water_interval = m_water_timer->getIntervalSeconds() / 60;
            m_state.temp_water_daily_goal = m_state.water_daily_goal;
        }
        if (m_standup_timer != nullptr) {
            m_state.temp_standup_interval = m_standup_timer->getIntervalSeconds() / 60;
            m_state.temp_standup_duration = m_standup_timer->getBreakDurationSeconds() / 60;
        }
        if (m_eye_care_timer != nullptr) {
            m_state.temp_eye_interval = m_eye_care_timer->getIntervalSeconds() / 60;
            m_state.temp_eye_break_duration = m_eye_care_timer->getBreakDurationSeconds();
        }
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(start_x + button_size + spacing);
    if (ImGui::Button(ICON_FA_ARROW_UP, ImVec2(button_size, button_size))) {
        if (m_callbacks.onToggleOverlayMode) {
            m_callbacks.onToggleOverlayMode();
        }
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(start_x + ((button_size + spacing) * 2.0f));
    if (ImGui::Button(ICON_FA_QUESTION_CIRCLE, ImVec2(button_size, button_size))) {
        m_state.show_help = true;
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(start_x + ((button_size + spacing) * 3.0f));
    if (ImGui::Button(ICON_FA_WINDOW_MINIMIZE, ImVec2(button_size, button_size))) {
        if (m_callbacks.onMinimizeToTray) {
            m_callbacks.onMinimizeToTray();
        }
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(start_x + ((button_size + spacing) * 4.0f));
    if (ImGui::Button(ICON_FA_POWER_OFF, ImVec2(button_size, button_size))) {
        if (m_callbacks.onRequestClose) {
            m_callbacks.onRequestClose();
        }
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);
    ImGui::Spacing();
}

void MainWindowView::renderEditTaskPopup() {
    if (m_state.show_edit_task) {
        ImGui::OpenPopup("Edit Task");
        m_state.show_edit_task = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(25.0f, 25.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

    if (ImGui::BeginPopupModal("Edit Task", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        constexpr float content_width = 450.0f;

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::Text("%s", m_state.edit_task_name.data());
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::SetCursorPosX(content_width - 5.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 0.3f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        if (ImGui::Button(ICON_FA_TIMES, ImVec2(40.0f, 40.0f))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::Text("Task Name");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::PushItemWidth(content_width - 50.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 12.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

        const bool enter_pressed =
            ImGui::InputText("##edit_taskname", m_state.edit_task_name.data(), m_state.edit_task_name.size(),
                             ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(2);
        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::Text("Act / Est Pomodoros");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        constexpr float input_width = 60.0f;
        constexpr float button_size = 35.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 12.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));

        const float row_start_x = ((content_width - (input_width * 2.0f + button_size * 4.0f + 30.0f)) * 0.5f) + 25.0f;
        ImGui::SetCursorPosX(row_start_x);

        if (ImGui::Button(ICON_FA_MINUS "##completed_minus", ImVec2(button_size, button_size))) {
            if (m_state.edit_task_completed_pomodoros > 0) {
                m_state.edit_task_completed_pomodoros--;
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(input_width);
        ImGui::InputInt("##completed", &m_state.edit_task_completed_pomodoros, 0, 0);
        ImGui::PopItemWidth();
        m_state.edit_task_completed_pomodoros = std::max(0, m_state.edit_task_completed_pomodoros);

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLUS "##completed_plus", ImVec2(button_size, button_size))) {
            if (m_state.edit_task_completed_pomodoros < m_state.edit_task_estimated_pomodoros) {
                m_state.edit_task_completed_pomodoros++;
            }
        }

        ImGui::SameLine();
        ImGui::Text("/");
        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_MINUS "##estimated_minus", ImVec2(button_size, button_size))) {
            if (m_state.edit_task_estimated_pomodoros > 1) {
                m_state.edit_task_estimated_pomodoros--;
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(input_width);
        ImGui::InputInt("##estimated", &m_state.edit_task_estimated_pomodoros, 0, 0);
        ImGui::PopItemWidth();
        m_state.edit_task_estimated_pomodoros = std::clamp(m_state.edit_task_estimated_pomodoros, 1, 20);

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLUS "##estimated_plus", ImVec2(button_size, button_size))) {
            if (m_state.edit_task_estimated_pomodoros < 20) {
                m_state.edit_task_estimated_pomodoros++;
            }
        }

        m_state.edit_task_completed_pomodoros =
            std::min(m_state.edit_task_completed_pomodoros, m_state.edit_task_estimated_pomodoros);

        ImGui::PopStyleColor(7);
        ImGui::PopStyleVar(2);

        ImGui::Spacing();
        ImGui::Spacing();

        constexpr float button_width = 100.0f;
        const float total_button_width = (button_width * 3.0f) + (ImGui::GetStyle().ItemSpacing.x * 2.0f);
        ImGui::SetCursorPosX(((content_width - total_button_width) * 0.5f) + 25.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        if (ImGui::Button((std::string(ICON_FA_TRASH) + "  Delete").c_str(), ImVec2(button_width, 40.0f))) {
            if (m_callbacks.onTaskRemoved && m_state.edit_task_index >= 0) {
                m_callbacks.onTaskRemoved(static_cast<size_t>(m_state.edit_task_index));
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor(4);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

        if (ImGui::Button("Cancel", ImVec2(button_width, 40.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor(4);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        const bool save_clicked = ImGui::Button("Save", ImVec2(button_width, 40.0f)) || enter_pressed;
        if (save_clicked && std::strlen(m_state.edit_task_name.data()) > 0 && m_state.edit_task_index >= 0) {
            if (m_callbacks.onTaskUpdated) {
                m_callbacks.onTaskUpdated(static_cast<size_t>(m_state.edit_task_index), m_state.edit_task_name.data(),
                                          m_state.edit_task_estimated_pomodoros, m_state.edit_task_completed_pomodoros);
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

void MainWindowView::renderHelpPopup() {
    if (m_state.show_help) {
        ImGui::OpenPopup("Help & Guide");
        m_state.show_help = false;
    }

    // Center the popup on screen
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(25.0f, 25.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

    if (ImGui::BeginPopupModal("Help & Guide", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoMove)) {
        // Use a dummy to establish minimum width, then get actual available width
        ImGui::Dummy(ImVec2(500.0f, 0.0f));
        const float content_width = ImGui::GetContentRegionAvail().x;

        // Title and close button on same line
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::SetCursorPosX((content_width - ImGui::CalcTextSize("Work Balance - Help & Guide").x) * 0.5f + 25.0f);
        ImGui::Text("Work Balance - Help & Guide");
        ImGui::PopStyleColor();

        // Close button in top-right corner (same line as title)
        ImGui::SameLine();
        ImGui::SetCursorPosX(content_width - 15.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 0.3f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        if (ImGui::Button(ICON_FA_TIMES "##help_close", ImVec2(40.0f, 40.0f))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Keyboard Shortcuts
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.35f, 0.35f, 1.0f));
        ImGui::Text(ICON_FA_KEYBOARD " Keyboard Shortcuts");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::BulletText("SPACE - Start/Pause the timer");
        ImGui::BulletText("UP ARROW - Toggle overlay mode");
        ImGui::Spacing();
        ImGui::Spacing();

        // Timer Modes
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.35f, 0.35f, 1.0f));
        ImGui::Text(ICON_FA_CLOCK " Timer Modes");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::BulletText("Pomodoro - Focus work session (default: 25 min)");
        ImGui::BulletText("Short Break - Quick rest period (default: 5 min)");
        ImGui::BulletText("Long Break - Extended rest period (default: 15 min)");
        ImGui::Spacing();
        ImGui::Spacing();

        // Managing Tasks
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.35f, 0.35f, 1.0f));
        ImGui::Text(ICON_FA_TASKS " Managing Tasks");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::BulletText("Click 'Add Task' to create a new task");
        ImGui::BulletText("Click the edit icon to modify task details");
        ImGui::BulletText("Check the box to mark a task as completed");
        ImGui::BulletText("Track pomodoros: Actual vs Estimated");
        ImGui::Spacing();
        ImGui::Spacing();

        // Wellness Features
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
        ImGui::Text(ICON_FA_TINT " Hydration Reminders");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::BulletText("Get periodic reminders to drink water");
        ImGui::BulletText("Track daily water intake with visual progress");
        ImGui::BulletText("Customize reminder intervals in Settings");
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.5f, 0.9f, 1.0f));
        ImGui::Text(ICON_FA_WALKING " Stand Up Reminders");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::BulletText("Reminds you to stand and stretch periodically");
        ImGui::BulletText("Customize interval and break duration");
        ImGui::BulletText("Helps reduce sedentary time during work");
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.6f, 1.0f));
        ImGui::Text(ICON_FA_EYE " Eye Care (20-20-20 Rule)");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::BulletText("Every 20 minutes, look at something 20 feet away");
        ImGui::BulletText("Hold focus for 20 seconds to reduce eye strain");
        ImGui::BulletText("Customize interval in Settings");
        ImGui::Spacing();
        ImGui::Spacing();

        // Overlay Mode
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.35f, 0.35f, 1.0f));
        ImGui::Text(ICON_FA_WINDOW_MAXIMIZE " Overlay Mode");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::BulletText("Compact timer view stays on top of other windows");
        ImGui::BulletText("Click and drag to reposition the overlay");
        ImGui::BulletText("Space still works to start/pause in overlay mode");
        ImGui::Spacing();
        ImGui::Spacing();

        // Settings
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.35f, 0.35f, 1.0f));
        ImGui::Text(ICON_FA_COG " Settings");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::BulletText("Click the gear icon to customize timer durations");
        ImGui::BulletText("Adjust Pomodoro, Short Break, and Long Break times");
        ImGui::BulletText("Configure wellness reminder intervals");
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::TextWrapped(
            "Work Balance helps you stay productive using the Pomodoro technique while caring for your health. "
            "Features include customizable timers, task management, overlay mode, and wellness reminders for "
            "hydration, movement, and eye care.");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::Text("Version 1.0.0");
        ImGui::PopStyleColor();

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

void MainWindowView::renderCurrentTask() {
    const auto tasks = m_task_manager.getTasks();
    if (tasks.empty() || std::cmp_greater_equal(m_state.current_task_index, tasks.size())) {
        return;
    }

    const auto& task = tasks[m_state.current_task_index];
    const std::string current_task = "#" + std::to_string(m_state.current_task_index + 1) + " " + task.name + " (" +
                                     std::to_string(task.completed_pomodoros) + "/" +
                                     std::to_string(task.estimated_pomodoros) + ")";

    const float window_width = ImGui::GetWindowSize().x;
    const float text_width = ImGui::CalcTextSize(current_task.c_str()).x;
    ImGui::SetCursorPosX((window_width - text_width) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.8f));
    ImGui::Text("%s", current_task.c_str());
    ImGui::PopStyleColor();

    ImGui::Spacing();
}

void MainWindowView::renderPomodoroCounter() const {
    ImGui::Spacing();

    const std::string counter_text = std::string(ICON_FA_CLOCK) +
                                     " Pomos: " + std::to_string(m_state.completed_pomodoros) + "/" +
                                     std::to_string(m_state.target_pomodoros);

    const float window_width = ImGui::GetWindowSize().x;
    const float text_width = ImGui::CalcTextSize(counter_text.c_str()).x;
    ImGui::SetCursorPosX((window_width - text_width) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.8f));
    ImGui::Text("%s", counter_text.c_str());
    ImGui::PopStyleColor();
}

void MainWindowView::handleWindowDragging() {
    const bool can_drag = ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive();
    updateWindowDragging(m_window.get(), can_drag, m_state.main_window_dragging, m_state.main_window_drag_offset, 5.0f);
}

} // namespace WorkBalance::App::UI
