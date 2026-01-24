#include "app/ui/components/TaskListPanel.h"
#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <string>

#include "assets/fonts/IconsFontAwesome5Pro.h"

namespace WorkBalance::App::UI::Components {

TaskListPanel::TaskListPanel(Core::TaskManager& taskManager, AppState& state, Callbacks callbacks)
    : m_task_manager(taskManager), m_state(state), m_callbacks(std::move(callbacks)) {
}

void TaskListPanel::render() {
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
            renderTaskItem(i, tasks[i]);
        }

        ImGui::Spacing();
        renderAddTaskButton();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    renderAddTaskPopup();
}

void TaskListPanel::renderTaskItem(size_t index, const Core::Task& task) {
    constexpr float task_item_height = 50.0f;
    constexpr float task_spacing = 8.0f;

    ImGui::PushID(static_cast<int>(index));

    const ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    const ImVec2 available_size = ImGui::GetContentRegionAvail();
    const float item_width = available_size.x;
    const float item_height = task_item_height;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImU32 bg_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.08f));
    draw_list->AddRectFilled(cursor_pos, ImVec2(cursor_pos.x + item_width, cursor_pos.y + item_height), bg_color, 8.0f);

    constexpr float left_padding = 16.0f;
    constexpr float top_padding = 12.0f;
    constexpr float right_padding = 16.0f;

    ImGui::SetCursorScreenPos(ImVec2(cursor_pos.x + left_padding, cursor_pos.y + top_padding));

    // Render checkbox
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
            ImVec2(checkbox_pos.x + checkbox_size + 1.0f, checkbox_pos.y + checkbox_size + 1.0f), checkbox_shadow_color,
            checkbox_rounding);

        checkbox_draw_list->AddRectFilled(
            ImVec2(checkbox_pos.x + 1.0f, checkbox_pos.y + 1.0f),
            ImVec2(checkbox_pos.x + checkbox_size - 1.0f, checkbox_pos.y + checkbox_size - 1.0f), checkbox_bg_color,
            checkbox_rounding);

        constexpr float check_padding = 5.0f;
        checkbox_draw_list->AddLine(
            ImVec2(checkbox_pos.x + check_padding + 1.0f, checkbox_pos.y + (checkbox_size / 2.0f) + 1.0f),
            ImVec2(checkbox_pos.x + (checkbox_size / 2.0f) + 1.0f,
                   checkbox_pos.y + checkbox_size - check_padding + 1.0f),
            checkbox_check_color, 3.5f);
        checkbox_draw_list->AddLine(
            ImVec2(checkbox_pos.x + (checkbox_size / 2.0f) + 1.0f,
                   checkbox_pos.y + checkbox_size - check_padding + 1.0f),
            ImVec2(checkbox_pos.x + checkbox_size - check_padding + 1.0f, checkbox_pos.y + check_padding + 1.0f),
            checkbox_check_color, 3.5f);
    } else {
        checkbox_draw_list->AddRectFilled(
            ImVec2(checkbox_pos.x + checkbox_shadow_offset, checkbox_pos.y + checkbox_shadow_offset),
            ImVec2(checkbox_pos.x + checkbox_size + checkbox_shadow_offset,
                   checkbox_pos.y + checkbox_size + checkbox_shadow_offset),
            checkbox_shadow_color, checkbox_rounding);

        checkbox_draw_list->AddRectFilled(checkbox_pos,
                                          ImVec2(checkbox_pos.x + checkbox_size, checkbox_pos.y + checkbox_size),
                                          checkbox_bg_color, checkbox_rounding);

        checkbox_draw_list->AddRect(checkbox_pos,
                                    ImVec2(checkbox_pos.x + checkbox_size, checkbox_pos.y + checkbox_size),
                                    checkbox_highlight_color, checkbox_rounding, 0, 1.5f);
    }

    const bool checkbox_clicked =
        ImGui::InvisibleButton(("##checkbox" + std::to_string(index)).c_str(), ImVec2(checkbox_size, checkbox_size));
    if (checkbox_clicked && m_callbacks.onTaskToggled) {
        m_callbacks.onTaskToggled(index);
    }

    // Task name
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

    // Progress indicator
    const std::string progress =
        std::to_string(task.completed_pomodoros) + "/" + std::to_string(task.estimated_pomodoros);

    const float progress_width = ImGui::CalcTextSize(progress.c_str()).x;
    constexpr float menu_button_width = 24.0f;

    ImGui::SetCursorScreenPos(
        ImVec2(cursor_pos.x + item_width - progress_width - menu_button_width - right_padding - 8.0f,
               cursor_pos.y + top_padding + 4.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
    ImGui::Text("%s", progress.c_str());
    ImGui::PopStyleColor();

    // Edit button
    ImGui::SetCursorScreenPos(ImVec2(cursor_pos.x + item_width - menu_button_width - 8.0f, cursor_pos.y + top_padding));

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    if (ImGui::Button(ICON_FA_PEN, ImVec2(20.0f, 20.0f))) {
        m_state.show_edit_task = true;
        m_state.edit_task_index = static_cast<int>(index);
        strncpy_s(m_state.edit_task_name.data(), m_state.edit_task_name.size(), task.name.c_str(), _TRUNCATE);
        m_state.edit_task_estimated_pomodoros = task.estimated_pomodoros;
        m_state.edit_task_completed_pomodoros = task.completed_pomodoros;
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    ImGui::SetCursorScreenPos(ImVec2(cursor_pos.x, cursor_pos.y + item_height + task_spacing));
    ImGui::PopID();
}

void TaskListPanel::renderAddTaskButton() {
    constexpr float add_task_height = 60.0f;

    const ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    const ImVec2 available_size = ImGui::GetContentRegionAvail();
    const float add_task_width = available_size.x;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImU32 border_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.3f));

    // Dashed border
    constexpr float dash_length = 8.0f;
    constexpr float gap_length = 4.0f;
    float current_x = cursor_pos.x;

    while (current_x < cursor_pos.x + add_task_width - dash_length) {
        draw_list->AddLine(ImVec2(current_x, cursor_pos.y), ImVec2(current_x + dash_length, cursor_pos.y), border_color,
                           1.0f);
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

void TaskListPanel::renderAddTaskPopup() {
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
        if (ImGui::Button(ICON_FA_TIMES "##addtask_close", ImVec2(40.0f, 40.0f))) {
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

        const bool enter_pressed = ImGui::InputText("##new_taskname", m_new_task_buffer, sizeof(m_new_task_buffer),
                                                    ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(2);
        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::Text("Est. Pomodoros");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        constexpr float input_width = 80.0f;
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

        const float row_start_x = ((content_width - (input_width + button_size * 2.0f + 16.0f)) * 0.5f) + 25.0f;
        ImGui::SetCursorPosX(row_start_x);

        if (ImGui::Button(ICON_FA_MINUS "##est_minus", ImVec2(button_size, button_size))) {
            if (m_new_task_estimated > 1) {
                m_new_task_estimated--;
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(input_width);
        ImGui::InputInt("##est_pomodoros", &m_new_task_estimated, 0, 0);
        ImGui::PopItemWidth();
        m_new_task_estimated = std::clamp(m_new_task_estimated, 1, 20);

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLUS "##est_plus", ImVec2(button_size, button_size))) {
            if (m_new_task_estimated < 20) {
                m_new_task_estimated++;
            }
        }

        ImGui::PopStyleColor(7);
        ImGui::PopStyleVar(2);

        ImGui::Spacing();
        ImGui::Spacing();

        constexpr float action_button_width = 100.0f;
        const float total_action_width = (action_button_width * 2.0f) + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(((content_width - total_action_width) * 0.5f) + 25.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

        if (ImGui::Button("Cancel", ImVec2(action_button_width, 40.0f))) {
            m_new_task_buffer[0] = '\0';
            m_new_task_estimated = 1;
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
        if (save_task && std::strlen(m_new_task_buffer) > 0) {
            if (m_callbacks.onTaskAdded) {
                m_callbacks.onTaskAdded(m_new_task_buffer, m_new_task_estimated);
            }
            m_new_task_buffer[0] = '\0';
            m_new_task_estimated = 1;
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

} // namespace WorkBalance::App::UI::Components
