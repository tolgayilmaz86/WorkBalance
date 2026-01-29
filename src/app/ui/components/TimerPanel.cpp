#include "app/ui/components/TimerPanel.h"
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <string>

#include "core/Configuration.h"
#include "ui/AppState.h"

namespace WorkBalance::App::UI::Components {

TimerPanel::TimerPanel(const Core::Timer& timer, AppState& state, App::ImGuiLayer& imgui, Callbacks callbacks)
    : m_timer(timer), m_state(state), m_imgui(imgui), m_callbacks(std::move(callbacks)) {
}

void TimerPanel::render() {
    const float window_width = ImGui::GetWindowSize().x;
    const float frame_width = std::min(600.0f, window_width - 40.0f);
    constexpr float frame_padding = 5.0f;

    ImGui::SetCursorPosX((window_width - frame_width) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 0.05f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(frame_padding, frame_padding));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.5f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));

    constexpr float frame_height = 320.0f;
    constexpr ImGuiWindowFlags timer_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    if (ImGui::BeginChild("TimerFrame", ImVec2(frame_width, frame_height), 1, timer_flags)) {
        ImGui::Spacing();
        renderModeButtons();
        renderTimerDisplay();
    }
    ImGui::EndChild();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
    ImGui::Spacing();
}

void TimerPanel::renderTimerOnly() {
    renderTimerDisplay();
}

void TimerPanel::renderModeButtons() {
    const float window_width = ImGui::GetWindowSize().x;
    constexpr float button_width = 120.0f;
    const float total_width = (button_width * 3.0f) + (ImGui::GetStyle().ItemSpacing.x * 2.0f);
    ImGui::SetCursorPosX((window_width - total_width) * 0.5f);

    const bool timer_running = m_timer.isRunning();

    auto render_button = [&](const char* label, Core::TimerMode mode) {
        const bool active = (m_timer.getCurrentMode() == mode);

        // Style: active mode is highlighted, other modes are dimmed if timer is running
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
        } else if (timer_running) {
            // Dim non-active buttons when timer is running (they're disabled)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.05f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.4f));
        }

        if (ImGui::Button(label, ImVec2(button_width, 40.0f))) {
            // Only change mode if timer is not running and mode is different
            if (!timer_running && !active && m_callbacks.onModeChange) {
                m_callbacks.onModeChange(mode);
            }
        }

        if (active) {
            ImGui::PopStyleColor();
        } else if (timer_running) {
            ImGui::PopStyleColor(2);
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

void TimerPanel::renderTimerDisplay() {
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

    if (button_pressed && m_callbacks.onToggle) {
        m_callbacks.onToggle();
    }

    ImGui::Spacing();
    ImGui::Spacing();
}

} // namespace WorkBalance::App::UI::Components
