#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <app/ui/MainWindowView.h>

#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>

#include "assets/fonts/IconsFontAwesome5Pro.h"
#include <core/Configuration.h>
#include <ui/AppState.h>

namespace WorkBalance::App::UI {
MainWindowView::MainWindowView(System::MainWindow& window, App::ImGuiLayer& imgui, Core::Timer& timer,
                               Core::TaskManager& taskManager, AppState& state, MainWindowCallbacks callbacks)
    : m_window(window), m_imgui(imgui), m_timer(timer), m_task_manager(taskManager), m_state(state),
      m_callbacks(std::move(callbacks)) {
}

void MainWindowView::render() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("WorkBalance", nullptr, window_flags)) {
        if (m_state.main_window_overlay_mode) {
            renderOverlayMode();
        } else {
            renderHeader();
            renderTimerFrame();
            renderCurrentTask();
            renderTaskList();
            renderPomodoroCounter();

            renderSettingsPopup();
            renderEditTaskPopup();
            renderHelpPopup();

            handleWindowDragging();
        }
    }
    ImGui::End();
}

void MainWindowView::renderOverlayMode() {
    const float window_width = ImGui::GetWindowSize().x;
    const float window_height = ImGui::GetWindowSize().y;

    std::string time_str = WorkBalance::TimeFormatter::formatTime(m_timer.getRemainingTime());

    ImFont* overlay_font = m_imgui.overlayFont();
    ImVec2 text_size = ImGui::CalcTextSize(time_str.c_str());

    if (overlay_font != nullptr) {
        ImGui::PushFont(overlay_font);
        text_size = ImGui::CalcTextSize(time_str.c_str());
        ImGui::PopFont();
    }

    ImGui::SetCursorPos(ImVec2((window_width - text_size.x) * 0.5f, (window_height - text_size.y) * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    if (overlay_font != nullptr) {
        ImGui::PushFont(overlay_font);
    }

    ImGui::Text("%s", time_str.c_str());

    if (overlay_font != nullptr) {
        ImGui::PopFont();
    }

    ImGui::PopStyleColor();

    GLFWwindow* handle = m_window.get();
    if (handle == nullptr) {
        return;
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
        m_state.main_overlay_dragging = true;

        int win_x = 0;
        int win_y = 0;
        glfwGetWindowPos(handle, &win_x, &win_y);

        double mouse_x = 0.0;
        double mouse_y = 0.0;
        glfwGetCursorPos(handle, &mouse_x, &mouse_y);

        mouse_x += win_x;
        mouse_y += win_y;

        m_state.main_overlay_drag_offset =
            ImVec2(static_cast<float>(mouse_x - win_x), static_cast<float>(mouse_y - win_y));
    }

    if (m_state.main_overlay_dragging) {
        if (ImGui::IsMouseDragging(0)) {
            int win_x = 0;
            int win_y = 0;
            glfwGetWindowPos(handle, &win_x, &win_y);

            double mouse_x = 0.0;
            double mouse_y = 0.0;
            glfwGetCursorPos(handle, &mouse_x, &mouse_y);

            mouse_x += win_x;
            mouse_y += win_y;

            const int new_x = static_cast<int>(mouse_x - m_state.main_overlay_drag_offset.x);
            const int new_y = static_cast<int>(mouse_y - m_state.main_overlay_drag_offset.y);
            glfwSetWindowPos(handle, new_x, new_y);
        } else if (ImGui::IsMouseReleased(0)) {
            m_state.main_overlay_dragging = false;
        }
    }
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
        m_state.temp_pomodoro_duration = m_timer.getPomodoroDuration() / 60;
        m_state.temp_short_break_duration = m_timer.getShortBreakDuration() / 60;
        m_state.temp_long_break_duration = m_timer.getLongBreakDuration() / 60;
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
    if (ImGui::Button(ICON_FA_POWER_OFF, ImVec2(button_size, button_size))) {
        if (m_callbacks.onRequestClose) {
            m_callbacks.onRequestClose();
        }
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);
    ImGui::Spacing();
}

void MainWindowView::renderSettingsPopup() {
    if (m_state.show_settings) {
        ImGui::OpenPopup("Settings");
        m_state.show_settings = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(25.0f, 25.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

    if (ImGui::BeginPopupModal("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        constexpr float content_width = 350.0f;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::SetCursorPosX(((content_width - ImGui::CalcTextSize("Settings").x) * 0.5f) + 25.0f);
        ImGui::Text("Settings");
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
        ImGui::Text("Timer (minutes)");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        constexpr float input_width = 60.0f;
        constexpr float button_size = 40.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 12.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));

        auto render_duration_row = [&](const char* label, const char* minus_id, const char* input_id,
                                       const char* plus_id, int& value, int min_value, int max_value) {
            ImGui::Text("%s", label);
            ImGui::Spacing();

            if (ImGui::Button((std::string(ICON_FA_MINUS) + minus_id).c_str(), ImVec2(button_size, button_size))) {
                if (value > min_value) {
                    value--;
                }
            }

            ImGui::SameLine();
            ImGui::PushItemWidth(input_width);
            ImGui::InputInt(input_id, &value, 0, 0);
            ImGui::PopItemWidth();
            value = std::clamp(value, min_value, max_value);

            ImGui::SameLine();
            if (ImGui::Button((std::string(ICON_FA_PLUS) + plus_id).c_str(), ImVec2(button_size, button_size))) {
                if (value < max_value) {
                    value++;
                }
            }

            ImGui::Spacing();
        };

        render_duration_row("Pomodoro", "##pomodoro_minus", "##pomodoro", "##pomodoro_plus",
                            m_state.temp_pomodoro_duration, 1, 60);
        render_duration_row("Short Break", "##shortbreak_minus", "##shortbreak", "##shortbreak_plus",
                            m_state.temp_short_break_duration, 1, 30);
        render_duration_row("Long Break", "##longbreak_minus", "##longbreak", "##longbreak_plus",
                            m_state.temp_long_break_duration, 1, 60);

        ImGui::PopStyleColor(7);
        ImGui::PopStyleVar(2);

        ImGui::Spacing();
        ImGui::Spacing();

        constexpr float button_width = 120.0f;
        ImGui::SetCursorPosX(((content_width - button_width) * 0.5f) + 25.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        if (ImGui::Button((std::string(ICON_FA_SAVE) + "  Save").c_str(), ImVec2(button_width, 40.0f))) {
            if (m_callbacks.onDurationsApplied) {
                m_callbacks.onDurationsApplied(m_state.temp_pomodoro_duration, m_state.temp_short_break_duration,
                                               m_state.temp_long_break_duration);
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
        ImGui::Text("%s", m_state.edit_task_name);
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
            ImGui::InputText("##edit_taskname", m_state.edit_task_name, sizeof(m_state.edit_task_name),
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

        constexpr float input_width = 150.0f;
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
        if (save_clicked && std::strlen(m_state.edit_task_name) > 0 && m_state.edit_task_index >= 0) {
            if (m_callbacks.onTaskUpdated) {
                m_callbacks.onTaskUpdated(static_cast<size_t>(m_state.edit_task_index), m_state.edit_task_name,
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

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(25.0f, 25.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

    if (ImGui::BeginPopupModal("Help & Guide", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        constexpr float content_width = 500.0f;

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::SetCursorPosX(((content_width - ImGui::CalcTextSize("Work Balance - Help & Guide").x) * 0.5f) + 25.0f);
        ImGui::Text("Work Balance - Help & Guide");
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

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.35f, 0.35f, 1.0f));
        ImGui::Text(ICON_FA_KEYBOARD " Keyboard Shortcuts");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::BulletText("SPACE - Start/Pause the timer");
        ImGui::BulletText("UP ARROW - Toggle overlay mode");
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.35f, 0.35f, 1.0f));
        ImGui::Text(ICON_FA_CLOCK " Timer Modes");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::BulletText("Pomodoro - Focus work session (default: 25 min)");
        ImGui::BulletText("Short Break - Quick rest period (default: 5 min)");
        ImGui::BulletText("Long Break - Extended rest period (default: 15 min)");
        ImGui::Spacing();
        ImGui::Spacing();

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

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.35f, 0.35f, 1.0f));
        ImGui::Text(ICON_FA_WINDOW_MAXIMIZE " Overlay Mode");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::BulletText("Compact timer view stays on top of other windows");
        ImGui::BulletText("Click and drag to reposition the overlay");
        ImGui::BulletText("Space still works to start/pause in overlay mode");
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.35f, 0.35f, 1.0f));
        ImGui::Text(ICON_FA_COG " Settings");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::BulletText("Click the gear icon to customize timer durations");
        ImGui::BulletText("Adjust Pomodoro, Short Break, and Long Break times");
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::TextWrapped("Work Balance helps you stay productive using the Pomodoro technique with a modern,"
                           " distraction-free interface. Customize timer durations, manage tasks, and use overlay"
                           " mode for focused work sessions.");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::Text("Version 1.0.0");
        ImGui::PopStyleColor();

        ImGui::EndPopup();
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

void MainWindowView::renderTaskList() {
    const float window_width = ImGui::GetWindowSize().x;
    const float panel_width = std::min(600.0f, window_width - 40.0f);
    ImGui::SetCursorPosX((window_width - panel_width) * 0.5f);

    const int task_count = static_cast<int>(m_task_manager.getTasks().size());
    constexpr float header_height = 30.0f;
    constexpr float task_spacing = 8.0f;
    constexpr float task_item_height = 50.0f;
    constexpr float add_task_height = 60.0f;
    constexpr float padding = 40.0f;
    constexpr float extra_bottom_spacing = 30.0f;

    const float total_height = padding + header_height + (task_count * (task_item_height + task_spacing)) +
                               add_task_height + extra_bottom_spacing;
    const float panel_height = std::max(400.0f, total_height);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 0.05f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));

    if (ImGui::BeginChild("TaskPanel", ImVec2(panel_width, panel_height), 1)) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.9f));
        ImGui::Text("Tasks");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        const auto tasks = m_task_manager.getTasks();
        for (size_t i = 0; i < tasks.size(); ++i) {
            const auto& task = tasks[i];
            ImGui::PushID(static_cast<int>(i));

            const ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
            const ImVec2 available_size = ImGui::GetContentRegionAvail();
            const float item_width = available_size.x;
            const float item_height = task_item_height;

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            const ImU32 bg_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.08f));
            draw_list->AddRectFilled(cursor_pos, ImVec2(cursor_pos.x + item_width, cursor_pos.y + item_height),
                                     bg_color, 8.0f);

            constexpr float left_padding = 16.0f;
            constexpr float top_padding = 12.0f;
            constexpr float right_padding = 16.0f;

            ImGui::SetCursorScreenPos(ImVec2(cursor_pos.x + left_padding, cursor_pos.y + top_padding));

            constexpr float checkbox_size = 24.0f;
            const ImVec2 checkbox_pos = ImGui::GetCursorScreenPos();
            ImDrawList* checkbox_draw_list = ImGui::GetWindowDrawList();

            const ImU32 checkbox_shadow_color = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
            const ImU32 checkbox_highlight_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.4f));
            const ImU32 checkbox_bg_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.95f));
            const ImU32 checkbox_check_color = ImGui::ColorConvertFloat4ToU32(m_state.background_color);

            constexpr float checkbox_rounding = 6.0f;
            constexpr float checkbox_shadow_offset = 2.0f;

            if (task.completed) {
                checkbox_draw_list->AddRectFilled(
                    ImVec2(checkbox_pos.x - 1.0f, checkbox_pos.y - 1.0f),
                    ImVec2(checkbox_pos.x + checkbox_size + 1.0f, checkbox_pos.y + checkbox_size + 1.0f),
                    checkbox_shadow_color, checkbox_rounding);

                checkbox_draw_list->AddRectFilled(
                    ImVec2(checkbox_pos.x + 1.0f, checkbox_pos.y + 1.0f),
                    ImVec2(checkbox_pos.x + checkbox_size - 1.0f, checkbox_pos.y + checkbox_size - 1.0f),
                    checkbox_bg_color, checkbox_rounding);

                constexpr float check_padding = 5.0f;
                checkbox_draw_list->AddLine(
                    ImVec2(checkbox_pos.x + check_padding + 1.0f, checkbox_pos.y + (checkbox_size / 2.0f) + 1.0f),
                    ImVec2(checkbox_pos.x + (checkbox_size / 2.0f) + 1.0f,
                           checkbox_pos.y + checkbox_size - check_padding + 1.0f),
                    checkbox_check_color, 3.5f);
                checkbox_draw_list->AddLine(ImVec2(checkbox_pos.x + (checkbox_size / 2.0f) + 1.0f,
                                                   checkbox_pos.y + checkbox_size - check_padding + 1.0f),
                                            ImVec2(checkbox_pos.x + checkbox_size - check_padding + 1.0f,
                                                   checkbox_pos.y + check_padding + 1.0f),
                                            checkbox_check_color, 3.5f);
            } else {
                checkbox_draw_list->AddRectFilled(
                    ImVec2(checkbox_pos.x + checkbox_shadow_offset, checkbox_pos.y + checkbox_shadow_offset),
                    ImVec2(checkbox_pos.x + checkbox_size + checkbox_shadow_offset,
                           checkbox_pos.y + checkbox_size + checkbox_shadow_offset),
                    checkbox_shadow_color, checkbox_rounding);

                checkbox_draw_list->AddRectFilled(
                    checkbox_pos, ImVec2(checkbox_pos.x + checkbox_size, checkbox_pos.y + checkbox_size),
                    checkbox_bg_color, checkbox_rounding);

                checkbox_draw_list->AddRect(checkbox_pos,
                                            ImVec2(checkbox_pos.x + checkbox_size, checkbox_pos.y + checkbox_size),
                                            checkbox_highlight_color, checkbox_rounding, 0, 1.5f);
            }

            const bool checkbox_clicked = ImGui::InvisibleButton(("##checkbox" + std::to_string(i)).c_str(),
                                                                 ImVec2(checkbox_size, checkbox_size));
            if (checkbox_clicked && m_callbacks.onTaskCompletionToggled) {
                m_callbacks.onTaskCompletionToggled(i);
            }

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, task.completed ? 0.6f : 0.9f));

            const ImVec2 text_pos = ImGui::GetCursorScreenPos();
            ImGui::Text("%s", task.name.c_str());

            if (task.completed) {
                const ImVec2 text_size = ImGui::CalcTextSize(task.name.c_str());
                ImDrawList* text_draw_list = ImGui::GetWindowDrawList();
                const ImU32 strikethrough_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.8f));
                const float line_y = text_pos.y + (text_size.y * 0.5f);
                text_draw_list->AddLine(ImVec2(text_pos.x, line_y), ImVec2(text_pos.x + text_size.x, line_y),
                                        strikethrough_color, 1.5f);
            }

            ImGui::PopStyleColor();

            std::stringstream progress_ss;
            progress_ss << task.completed_pomodoros << "/" << task.estimated_pomodoros;
            const std::string progress = progress_ss.str();

            const float progress_width = ImGui::CalcTextSize(progress.c_str()).x;
            constexpr float menu_button_width = 24.0f;

            ImGui::SetCursorScreenPos(
                ImVec2(cursor_pos.x + item_width - progress_width - menu_button_width - right_padding - 8.0f,
                       cursor_pos.y + top_padding + 4.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
            ImGui::Text("%s", progress.c_str());
            ImGui::PopStyleColor();

            ImGui::SetCursorScreenPos(
                ImVec2(cursor_pos.x + item_width - menu_button_width - 8.0f, cursor_pos.y + top_padding));

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

            if (ImGui::Button(ICON_FA_PEN, ImVec2(20.0f, 20.0f))) {
                m_state.show_edit_task = true;
                m_state.edit_task_index = static_cast<int>(i);
                strncpy_s(m_state.edit_task_name, task.name.c_str(), sizeof(m_state.edit_task_name) - 1);
                m_state.edit_task_estimated_pomodoros = task.estimated_pomodoros;
                m_state.edit_task_completed_pomodoros = task.completed_pomodoros;
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);

            ImGui::SetCursorScreenPos(ImVec2(cursor_pos.x, cursor_pos.y + item_height + task_spacing));
            ImGui::PopID();
        }

        ImGui::Spacing();

        const ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
        const ImVec2 available_size = ImGui::GetContentRegionAvail();
        const float add_task_width = available_size.x;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImU32 border_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.3f));

        constexpr float dash_length = 8.0f;
        constexpr float gap_length = 4.0f;
        float current_x = cursor_pos.x;

        while (current_x < cursor_pos.x + add_task_width - dash_length) {
            draw_list->AddLine(ImVec2(current_x, cursor_pos.y), ImVec2(current_x + dash_length, cursor_pos.y),
                               border_color, 1.0f);
            current_x += dash_length + gap_length;
        }

        current_x = cursor_pos.x;
        while (current_x < cursor_pos.x + add_task_width - dash_length) {
            draw_list->AddLine(ImVec2(current_x, cursor_pos.y + add_task_height),
                               ImVec2(current_x + dash_length, cursor_pos.y + add_task_height), border_color, 1.0f);
            current_x += dash_length + gap_length;
        }

        draw_list->AddLine(ImVec2(cursor_pos.x, cursor_pos.y), ImVec2(cursor_pos.x, cursor_pos.y + add_task_height),
                           border_color, 1.0f);
        draw_list->AddLine(ImVec2(cursor_pos.x + add_task_width, cursor_pos.y),
                           ImVec2(cursor_pos.x + add_task_width, cursor_pos.y + add_task_height), border_color, 1.0f);

        ImGui::SetCursorScreenPos(cursor_pos);
        if (ImGui::InvisibleButton("AddTaskButton", ImVec2(add_task_width, add_task_height))) {
            m_state.show_add_task = true;
        }

        const std::string add_text = std::string(ICON_FA_PLUS) + "  Add Task";
        const ImVec2 text_size = ImGui::CalcTextSize(add_text.c_str());
        const ImVec2 text_pos = ImVec2(cursor_pos.x + ((add_task_width - text_size.x) * 0.5f),
                                       cursor_pos.y + ((add_task_height - text_size.y) * 0.5f));

        ImGui::SetCursorScreenPos(text_pos);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
        ImGui::Text("%s", add_text.c_str());
        ImGui::PopStyleColor();

        ImGui::SetCursorScreenPos(ImVec2(cursor_pos.x, cursor_pos.y + add_task_height));
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    if (m_state.show_add_task) {
        ImGui::OpenPopup("Add Task");
        m_state.show_add_task = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(25.0f, 25.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

    if (ImGui::BeginPopupModal("Add Task", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        constexpr float content_width = 400.0f;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::SetCursorPosX(((content_width - ImGui::CalcTextSize("Add Task").x) * 0.5f) + 25.0f);
        ImGui::Text("Add Task");
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

        static char task_buffer[256] = "";
        static int est_pomodoros = 1;

        ImGui::PushItemWidth(content_width - 50.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 12.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

        const bool enter_pressed =
            ImGui::InputText("##taskname", task_buffer, sizeof(task_buffer), ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(2);
        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::Text("Estimated Pomodoros");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        constexpr float input_width_est = 100.0f;
        constexpr float button_size_est = 40.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 12.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

        ImGui::SetCursorPosX(((content_width - (input_width_est + button_size_est * 2.0f + 20.0f)) * 0.5f) + 25.0f);

        if (ImGui::Button(ICON_FA_MINUS "##add_estimated_minus", ImVec2(button_size_est, button_size_est))) {
            if (est_pomodoros > 1) {
                est_pomodoros--;
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(input_width_est);
        ImGui::InputInt("##add_estimated", &est_pomodoros, 0, 0);
        ImGui::PopItemWidth();
        est_pomodoros = std::clamp(est_pomodoros, 1, 20);

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLUS "##add_estimated_plus", ImVec2(button_size_est, button_size_est))) {
            if (est_pomodoros < 20) {
                est_pomodoros++;
            }
        }

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(2);

        ImGui::Spacing();
        ImGui::Spacing();

        constexpr float action_button_width = 120.0f;
        const float total_action_width = (action_button_width * 2.0f) + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(((content_width - total_action_width) * 0.5f) + 25.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

        if (ImGui::Button("Cancel", ImVec2(action_button_width, 40.0f))) {
            task_buffer[0] = '\0';
            est_pomodoros = 1;
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor(4);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        const bool save_task =
            ImGui::Button((std::string(ICON_FA_SAVE) + "  Save").c_str(), ImVec2(action_button_width, 40.0f)) ||
            enter_pressed;
        if (save_task && std::strlen(task_buffer) > 0) {
            if (m_callbacks.onTaskAdded) {
                m_callbacks.onTaskAdded(task_buffer, est_pomodoros);
            }
            task_buffer[0] = '\0';
            est_pomodoros = 1;
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

void MainWindowView::renderModeButtons() {
    const float window_width = ImGui::GetWindowSize().x;
    constexpr float button_width = 120.0f;
    const float total_width = (button_width * 3.0f) + (ImGui::GetStyle().ItemSpacing.x * 2.0f);
    ImGui::SetCursorPosX((window_width - total_width) * 0.5f);

    auto render_button = [&](const char* label, Core::TimerMode mode) {
        const bool active = (m_timer.getCurrentMode() == mode);
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
        }

        if (ImGui::Button(label, ImVec2(button_width, 40.0f))) {
            if (m_callbacks.onModeChange) {
                m_callbacks.onModeChange(mode);
            }
        }

        if (active) {
            ImGui::PopStyleColor();
        }
    };

    render_button("Pomodoro", Core::TimerMode::Pomodoro);
    ImGui::SameLine();
    render_button("Short Break", Core::TimerMode::ShortBreak);
    ImGui::SameLine();
    render_button("Long Break", Core::TimerMode::LongBreak);

    ImGui::Spacing();
    ImGui::Spacing();
}

void MainWindowView::renderTimer() {
    const std::string time_str = WorkBalance::TimeFormatter::formatTime(m_timer.getRemainingTime());

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ImFont* timer_font = m_imgui.timerFont();
    if (timer_font != nullptr) {
        ImGui::PushFont(timer_font);
    }

    const float window_width = ImGui::GetWindowSize().x;
    const float text_width = ImGui::CalcTextSize(time_str.c_str()).x;
    ImGui::SetCursorPosX((window_width - text_width) * 0.5f);
    ImGui::Text("%s", time_str.c_str());

    if (timer_font != nullptr) {
        ImGui::PopFont();
    }

    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Spacing();

    constexpr float button_width = 240.0f;
    constexpr float button_height = 60.0f;
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);

    const ImVec2 button_pos = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const bool is_running = (m_timer.getState() == Core::TimerState::Running);
    const std::string button_text = is_running ? "PAUSE" : "START";

    ImFont* button_font = m_imgui.buttonFont();
    float text_width_btn = ImGui::CalcTextSize(button_text.c_str()).x;
    float text_height_btn = ImGui::CalcTextSize(button_text.c_str()).y;
    if (button_font != nullptr) {
        ImGui::PushFont(button_font);
        text_width_btn = ImGui::CalcTextSize(button_text.c_str()).x;
        text_height_btn = ImGui::CalcTextSize(button_text.c_str()).y;
        ImGui::PopFont();
    }

    const ImU32 shadow_color = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
    const ImU32 highlight_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.4f));
    const ImU32 button_bg_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.95f));

    constexpr float rounding = 8.0f;
    constexpr float shadow_offset = 5.0f;

    if (is_running) {
        draw_list->AddRectFilled(ImVec2(button_pos.x - 1.0f, button_pos.y - 1.0f),
                                 ImVec2(button_pos.x + button_width + 1.0f, button_pos.y + button_height + 1.0f),
                                 shadow_color, rounding);
        draw_list->AddRectFilled(ImVec2(button_pos.x + 2.0f, button_pos.y + 2.0f),
                                 ImVec2(button_pos.x + button_width, button_pos.y + button_height), button_bg_color,
                                 rounding);
    } else {
        draw_list->AddRectFilled(
            ImVec2(button_pos.x + shadow_offset, button_pos.y + shadow_offset),
            ImVec2(button_pos.x + button_width + shadow_offset, button_pos.y + button_height + shadow_offset),
            shadow_color, rounding);
        draw_list->AddRectFilled(button_pos, ImVec2(button_pos.x + button_width, button_pos.y + button_height),
                                 button_bg_color, rounding);
        draw_list->AddRect(button_pos, ImVec2(button_pos.x + button_width, button_pos.y + button_height),
                           highlight_color, rounding, 0, 2.0f);
    }

    const bool button_pressed = ImGui::InvisibleButton("StartPauseButton", ImVec2(button_width, button_height));

    const ImVec2 text_pos =
        ImVec2(button_pos.x + ((button_width - text_width_btn) * 0.5f) + (is_running ? 2.0f : 0.0f),
               button_pos.y + ((button_height - text_height_btn) * 0.5f) + (is_running ? 2.0f : 0.0f));

    const ImU32 text_color = ImGui::ColorConvertFloat4ToU32(m_state.background_color);

    if (button_font != nullptr) {
        draw_list->AddText(button_font, Core::Configuration::BUTTON_FONT_SIZE, text_pos, text_color,
                           button_text.c_str());
    } else {
        draw_list->AddText(text_pos, text_color, button_text.c_str());
    }

    if (button_pressed && m_callbacks.onToggleTimer) {
        m_callbacks.onToggleTimer();
    }

    ImGui::Spacing();
    ImGui::Spacing();
}

void MainWindowView::renderTimerFrame() {
    const float window_width = ImGui::GetWindowSize().x;
    const float frame_width = std::min(600.0f, window_width - 40.0f);
    constexpr float frame_padding = 35.0f;

    ImGui::SetCursorPosX((window_width - frame_width) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 0.05f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(frame_padding, frame_padding));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.5f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));

    constexpr float frame_height = 360.0f;
    if (ImGui::BeginChild("TimerFrame", ImVec2(frame_width, frame_height), 1, ImGuiWindowFlags_NoScrollbar)) {
        ImGui::Spacing();
        renderModeButtons();
        renderTimer();
        ImGui::Spacing();
    }
    ImGui::EndChild();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
    ImGui::Spacing();
}

void MainWindowView::renderCurrentTask() {
    const auto tasks = m_task_manager.getTasks();
    if (tasks.empty() || m_state.current_task_index >= static_cast<int>(tasks.size())) {
        return;
    }

    std::stringstream ss;
    ss << "#" << (m_state.current_task_index + 1) << " " << tasks[m_state.current_task_index].name;
    const std::string current_task = ss.str();

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

    std::stringstream counter_ss;
    counter_ss << ICON_FA_CLOCK << " Pomos: " << m_state.completed_pomodoros << "/" << m_state.target_pomodoros;
    const std::string counter_text = counter_ss.str();

    const float window_width = ImGui::GetWindowSize().x;
    const float text_width = ImGui::CalcTextSize(counter_text.c_str()).x;
    ImGui::SetCursorPosX((window_width - text_width) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.8f));
    ImGui::Text("%s", counter_text.c_str());
    ImGui::PopStyleColor();
}

void MainWindowView::handleWindowDragging() {
    GLFWwindow* handle = m_window.get();
    if (handle == nullptr) {
        return;
    }

    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive()) {
        if (ImGui::IsMouseClicked(0)) {
            m_state.main_window_dragging = true;

            int win_x = 0;
            int win_y = 0;
            glfwGetWindowPos(handle, &win_x, &win_y);

            double mouse_x = 0.0;
            double mouse_y = 0.0;
            glfwGetCursorPos(handle, &mouse_x, &mouse_y);

            mouse_x += win_x;
            mouse_y += win_y;

            m_state.main_window_drag_offset =
                ImVec2(static_cast<float>(mouse_x - win_x), static_cast<float>(mouse_y - win_y));
        }
    }

    if (m_state.main_window_dragging) {
        if (ImGui::IsMouseDragging(0, 5.0f)) {
            int win_x = 0;
            int win_y = 0;
            glfwGetWindowPos(handle, &win_x, &win_y);

            double mouse_x = 0.0;
            double mouse_y = 0.0;
            glfwGetCursorPos(handle, &mouse_x, &mouse_y);

            mouse_x += win_x;
            mouse_y += win_y;

            const int new_x = static_cast<int>(mouse_x - m_state.main_window_drag_offset.x);
            const int new_y = static_cast<int>(mouse_y - m_state.main_window_drag_offset.y);
            glfwSetWindowPos(handle, new_x, new_y);
        }

        if (ImGui::IsMouseReleased(0)) {
            m_state.main_window_dragging = false;
        }
    }
}

} // namespace WorkBalance::App::UI
