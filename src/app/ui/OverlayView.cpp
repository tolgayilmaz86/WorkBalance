#include <app/ui/OverlayView.h>

#include <GLFW/glfw3.h>

#include <string>

#include "core/Configuration.h"

namespace WorkBalance::App::UI {
void OverlayView::renderContent(System::OverlayWindow& overlay_window) {
    const ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                           ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    const auto [overlay_width, overlay_height] = overlay_window.getFramebufferSize();
    ImGui::SetNextWindowPos(m_state.overlay_position, ImGuiCond_FirstUseEver);
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

        std::string time_str = WorkBalance::TimeFormatter::formatTime(m_timer.getRemainingTime());
        ImVec2 text_size = ImGui::CalcTextSize(time_str.c_str());

        constexpr float text_scale = 2.0f;
        text_size.x *= text_scale;
        text_size.y *= text_scale;

        ImGui::SetCursorPos(ImVec2((window_width - text_size.x) * 0.5f, (window_height - text_size.y) * 0.5f));

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::SetWindowFontScale(text_scale);
        ImGui::Text("%s", time_str.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

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

    m_imgui.render();

    glDisable(GL_BLEND);
}

} // namespace WorkBalance::App::UI
