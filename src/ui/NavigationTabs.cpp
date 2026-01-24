#include <ui/NavigationTabs.h>
#include <core/Configuration.h>
#include <core/WellnessTypes.h>
#include "assets/fonts/IconsFontAwesome5Pro.h"

namespace WorkBalance::UI {

NavigationTabs::NavigationTabs(AppState& state, NavigationCallbacks callbacks)
    : m_state(state), m_callbacks(std::move(callbacks)) {
}

void NavigationTabs::render(const ImVec2& /*viewport_pos*/, const ImVec2& /*viewport_size*/, float /*panel_height*/) {
    // This now renders inline tabs (button style) within the current ImGui window
    renderInline();
}

void NavigationTabs::renderInline() {
    const float window_width = ImGui::GetWindowSize().x;
    constexpr int num_tabs = 4;
    const float total_width = (TAB_SIZE * num_tabs) + (TAB_SPACING * (num_tabs - 1));
    const float start_x = (window_width - total_width) * 0.5f;

    ImGui::SetCursorPosX(start_x);

    // Style tabs like the header buttons but bigger
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.8f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, TAB_ROUNDING);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(TAB_SPACING, 0.0f));

    renderTabButton(NavigationTab::Pomodoro, ICON_FA_CLOCK, "Pomodoro Timer", Core::Configuration::POMODORO_BG_COLOR);

    ImGui::SameLine();
    renderTabButton(NavigationTab::Water, ICON_FA_TINT, "Hydration Reminder", Core::WellnessDefaults::WATER_BG_COLOR);

    ImGui::SameLine();
    renderTabButton(NavigationTab::Standup, ICON_FA_WALKING, "Stand Up Reminder",
                    Core::WellnessDefaults::STANDUP_BG_COLOR);

    ImGui::SameLine();
    renderTabButton(NavigationTab::EyeCare, ICON_FA_EYE, "Eye Care (20-20-20)",
                    Core::WellnessDefaults::EYE_STRAIN_BG_COLOR);

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(4);

    ImGui::Spacing();
}

void NavigationTabs::renderTabButton(NavigationTab tab, const char* icon, const char* tooltip,
                                     const ImVec4& active_color) {
    const bool is_active = (m_state.active_tab == tab);

    // Create unique ID for this button
    ImGui::PushID(static_cast<int>(tab));

    // If active, use the tab's color as background
    if (is_active) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(active_color.x, active_color.y, active_color.z, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(active_color.x, active_color.y, active_color.z, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(active_color.x, active_color.y, active_color.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    if (ImGui::Button(icon, ImVec2(TAB_SIZE, TAB_SIZE))) {
        if (m_state.active_tab != tab) {
            m_state.active_tab = tab;
            if (m_callbacks.onTabChanged) {
                m_callbacks.onTabChanged(tab);
            }
        }
    }

    if (is_active) {
        ImGui::PopStyleColor(4);
    }

    // Tooltip on hover
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(tooltip);
        ImGui::EndTooltip();
    }

    ImGui::PopID();
}

} // namespace WorkBalance::UI
