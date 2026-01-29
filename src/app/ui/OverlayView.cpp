#include <app/ui/OverlayView.h>

#include <GLFW/glfw3.h>

#include <string>

#include "core/Configuration.h"

namespace WorkBalance::App::UI {
void OverlayView::renderContent(System::OverlayWindow& overlay_window) {
    const ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                           ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

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

    // Calculate required window size based on text
    ImFont* overlay_font = m_imgui.overlayFont();
    const float font_scale = (active_wellness_count > 0) ? 0.7f : 1.0f;

    ImGui::PushFont(overlay_font);
    ImGui::SetWindowFontScale(font_scale);
    ImVec2 text_size = ImGui::CalcTextSize(display_str.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();

    constexpr float padding_x = 40.0f; // Horizontal padding
    constexpr float padding_y = 20.0f; // Vertical padding
    const int required_width = static_cast<int>(text_size.x + padding_x);
    const int required_height = static_cast<int>(text_size.y + padding_y);

    // Resize window if needed
    const auto [current_width, current_height] = overlay_window.getFramebufferSize();
    if (current_width != required_width || current_height != required_height) {
        overlay_window.setSize(required_width, required_height);
    }

    const auto [overlay_width, overlay_height] = overlay_window.getFramebufferSize();
    // ImGui window fills the entire GLFW overlay window - actual position is handled by GLFW
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(overlay_width), static_cast<float>(overlay_height)));

    ImVec4 overlay_bg = m_state.background_color;
    overlay_bg.w = 0.95f;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, overlay_bg);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 5.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

    if (ImGui::Begin("Timer Overlay Window", nullptr, overlay_flags)) {
        const float window_width = ImGui::GetWindowSize().x;
        const float window_height = ImGui::GetWindowSize().y;

        // Use overlay font which has icons merged
        ImGui::PushFont(overlay_font);
        ImGui::SetWindowFontScale(font_scale);

        ImGui::SetCursorPos(ImVec2((window_width - text_size.x) * 0.5f, (window_height - text_size.y) * 0.5f));

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::Text("%s", display_str.c_str());
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopFont();

        GLFWwindow* handle = overlay_window.get();
        if (handle != nullptr) {
            if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
                m_state.overlay_dragging = true;

                int win_x = 0;
                int win_y = 0;
                glfwGetWindowPos(handle, &win_x, &win_y);

                double mouse_x = 0.0;
                double mouse_y = 0.0;
                glfwGetCursorPos(handle, &mouse_x, &mouse_y);

                mouse_x += win_x;
                mouse_y += win_y;

                m_state.overlay_drag_offset =
                    ImVec2(static_cast<float>(mouse_x - win_x), static_cast<float>(mouse_y - win_y));
            }

            if (m_state.overlay_dragging) {
                if (ImGui::IsMouseDragging(0)) {
                    int win_x = 0;
                    int win_y = 0;
                    glfwGetWindowPos(handle, &win_x, &win_y);

                    double mouse_x = 0.0;
                    double mouse_y = 0.0;
                    glfwGetCursorPos(handle, &mouse_x, &mouse_y);

                    mouse_x += win_x;
                    mouse_y += win_y;

                    const int new_x = static_cast<int>(mouse_x - m_state.overlay_drag_offset.x);
                    const int new_y = static_cast<int>(mouse_y - m_state.overlay_drag_offset.y);
                    glfwSetWindowPos(handle, new_x, new_y);
                    m_state.overlay_position = ImVec2(static_cast<float>(new_x), static_cast<float>(new_y));
                } else if (ImGui::IsMouseReleased(0)) {
                    m_state.overlay_dragging = false;
                }
            }
        }
    }
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void OverlayView::renderFrame(System::OverlayWindow& overlay_window) {
    const auto [width, height] = overlay_window.getFramebufferSize();
    glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ImGuiLayer::render();

    glDisable(GL_BLEND);
}

} // namespace WorkBalance::App::UI
