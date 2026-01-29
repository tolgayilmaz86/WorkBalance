#include "app/ui/components/SettingsPopup.h"
#include <algorithm>
#include <imgui.h>
#include <string>

// Font Awesome icons
#include "assets/fonts/IconsFontAwesome5Pro.h"
#include "system/WindowsStartup.h"

namespace WorkBalance::App::UI::Components {

namespace {
// Tab styling colors
constexpr ImVec4 TAB_COLOR = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
constexpr ImVec4 TAB_HOVERED = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
constexpr ImVec4 TAB_ACTIVE = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
constexpr ImVec4 TAB_TEXT = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
constexpr float CONTENT_WIDTH = 380.0f;
constexpr float TAB_CONTENT_HEIGHT = 600.0f;
} // namespace

SettingsPopup::SettingsPopup(AppState& state, Callbacks callbacks) : m_state(state), m_callbacks(std::move(callbacks)) {
}

void SettingsPopup::render() {
    if (m_state.show_settings) {
        ImGui::OpenPopup("Settings");
        m_state.show_settings = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

    if (ImGui::BeginPopupModal("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        // Title
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::SetCursorPosX(((CONTENT_WIDTH - ImGui::CalcTextSize("Settings").x) * 0.5f) + 20.0f);
        ImGui::Text("Settings");
        ImGui::PopStyleColor();

        // Close button
        ImGui::SameLine();
        ImGui::SetCursorPosX(CONTENT_WIDTH - 10.0f);
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

        // Render the tab bar with content
        renderTabBar();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Render save button
        renderButtons();

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

void SettingsPopup::renderTabBar() {
    // Style the tab bar
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 6.0f);
    ImGui::PushStyleColor(ImGuiCol_Tab, TAB_COLOR);
    ImGui::PushStyleColor(ImGuiCol_TabHovered, TAB_HOVERED);
    ImGui::PushStyleColor(ImGuiCol_TabSelected, TAB_ACTIVE);
    ImGui::PushStyleColor(ImGuiCol_TabSelectedOverline, ImVec4(0.3f, 0.6f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, TAB_TEXT);

    if (ImGui::BeginTabBar("SettingsTabBar", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem((std::string(ICON_FA_CLOCK) + " Timer").c_str())) {
            ImGui::BeginChild("TimerContent", ImVec2(CONTENT_WIDTH, TAB_CONTENT_HEIGHT), ImGuiChildFlags_None);
            ImGui::Spacing();
            renderPomodoroTab();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem((std::string(ICON_FA_HEART) + " Wellness").c_str())) {
            ImGui::BeginChild("WellnessContent", ImVec2(CONTENT_WIDTH, TAB_CONTENT_HEIGHT), ImGuiChildFlags_None);
            ImGui::Spacing();
            renderWellnessTab();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem((std::string(ICON_FA_VOLUME_UP) + " Sound").c_str())) {
            ImGui::BeginChild("SoundContent", ImVec2(CONTENT_WIDTH, TAB_CONTENT_HEIGHT), ImGuiChildFlags_None);
            ImGui::Spacing();
            renderSoundTab();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem((std::string(ICON_FA_COG) + " General").c_str())) {
            ImGui::BeginChild("GeneralContent", ImVec2(CONTENT_WIDTH, TAB_CONTENT_HEIGHT), ImGuiChildFlags_None);
            ImGui::Spacing();
            renderGeneralTab();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(2);
}

void SettingsPopup::renderPomodoroTab() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::Text(ICON_FA_STOPWATCH " Timer Durations (minutes)");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 12.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));

    renderDurationRow("Pomodoro", "##pomodoro_minus", "##pomodoro", "##pomodoro_plus", m_state.temp_pomodoro_duration,
                      1, 60);
    renderDurationRow("Short Break", "##shortbreak_minus", "##shortbreak", "##shortbreak_plus",
                      m_state.temp_short_break_duration, 1, 30);
    renderDurationRow("Long Break", "##longbreak_minus", "##longbreak", "##longbreak_plus",
                      m_state.temp_long_break_duration, 1, 60);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::Text(ICON_FA_SYNC " Pomodoro Cycle");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Spacing();

    renderDurationRow("Pomodoros before long break", "##pom_cycle_minus", "##pom_cycle", "##pom_cycle_plus",
                      m_state.pomodoros_before_long_break, 1, 10);
    renderDurationRow("Long breaks per cycle", "##long_breaks_minus", "##long_breaks", "##long_breaks_plus",
                      m_state.long_breaks_in_cycle, 1, 5);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Checkbox("Auto-start breaks after pomodoro", &m_state.auto_start_breaks);
    ImGui::Checkbox("Auto-start pomodoro after break", &m_state.auto_start_pomodoros);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::Text(ICON_FA_DESKTOP " Overlay");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Checkbox("Show timer in overlay", &m_state.show_pomodoro_in_overlay);

    ImGui::PopStyleColor(7);
    ImGui::PopStyleVar(2);
}

void SettingsPopup::renderWellnessTab() {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 12.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));

    // Water settings
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
    ImGui::Text(ICON_FA_TINT " Water Reminders");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    renderDurationRow("Interval (min)", "##water_minus", "##water_interval", "##water_plus",
                      m_state.temp_water_interval, 5, 120);
    renderDurationRow("Daily Goal", "##watergoal_minus", "##water_goal", "##watergoal_plus",
                      m_state.temp_water_daily_goal, 1, 20);
    ImGui::Checkbox("Show in overlay##water", &m_state.show_water_in_overlay);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Standup settings
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.5f, 0.9f, 1.0f));
    ImGui::Text(ICON_FA_WALKING " Stand Up Reminders");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    renderDurationRow("Interval (min)", "##standup_int_minus", "##standup_interval", "##standup_int_plus",
                      m_state.temp_standup_interval, 15, 120);
    renderDurationRow("Break (min)", "##standup_dur_minus", "##standup_duration", "##standup_dur_plus",
                      m_state.temp_standup_duration, 1, 15);
    ImGui::Checkbox("Show in overlay##standup", &m_state.show_standup_in_overlay);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Eye care settings
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.6f, 1.0f));
    ImGui::Text(ICON_FA_EYE " Eye Care (20-20-20)");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    renderDurationRow("Interval (min)", "##eye_int_minus", "##eye_interval", "##eye_int_plus",
                      m_state.temp_eye_interval, 10, 60);
    renderDurationRow("Break (sec)", "##eye_dur_minus", "##eye_duration", "##eye_dur_plus",
                      m_state.temp_eye_break_duration, 10, 60);
    ImGui::Checkbox("Show in overlay##eyecare", &m_state.show_eye_care_in_overlay);

    ImGui::PopStyleColor(7);
    ImGui::PopStyleVar(2);
}

void SettingsPopup::renderGeneralTab() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::Text(ICON_FA_WINDOW_MAXIMIZE " Windows Startup");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Spacing();

    // Start with Windows checkbox - applies immediately when changed
    if (ImGui::Checkbox("Start with Windows", &m_state.start_with_windows)) {
        System::WindowsStartup::setStartupEnabled(m_state.start_with_windows);
    }
    ImGui::Spacing();

    ImGui::Checkbox("Start minimized to system tray", &m_state.start_minimized);
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::TextWrapped("When 'Start minimized' is enabled, the application will start minimized to the system tray "
                       "when launched automatically at Windows startup. Manual launch always shows the window.");
    ImGui::PopStyleColor();
}

void SettingsPopup::renderSoundTab() {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.4f, 0.6f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));

    // Pomodoro Timer Sound
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.35f, 0.35f, 1.0f));
    ImGui::Text(ICON_FA_CLOCK " Pomodoro Timer");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Checkbox("Enable sound##pomodoro", &m_state.pomodoro_sound_enabled);
    if (m_state.pomodoro_sound_enabled) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f);
        ImGui::SliderInt("Volume##pomodoro", &m_state.pomodoro_sound_volume, 0, 100, "%d%%");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Water Reminder Sound
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
    ImGui::Text(ICON_FA_TINT " Water Reminder");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Checkbox("Enable sound##water", &m_state.water_sound_enabled);
    if (m_state.water_sound_enabled) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f);
        ImGui::SliderInt("Volume##water", &m_state.water_sound_volume, 0, 100, "%d%%");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Standup Reminder Sound
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.5f, 0.9f, 1.0f));
    ImGui::Text(ICON_FA_WALKING " Stand Up Reminder");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Checkbox("Enable sound##standup", &m_state.standup_sound_enabled);
    if (m_state.standup_sound_enabled) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f);
        ImGui::SliderInt("Volume##standup", &m_state.standup_sound_volume, 0, 100, "%d%%");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Eye Care Reminder Sound
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.6f, 1.0f));
    ImGui::Text(ICON_FA_EYE " Eye Care Reminder");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Checkbox("Enable sound##eyecare", &m_state.eye_care_sound_enabled);
    if (m_state.eye_care_sound_enabled) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f);
        ImGui::SliderInt("Volume##eyecare", &m_state.eye_care_sound_volume, 0, 100, "%d%%");
    }

    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(2);
}

void SettingsPopup::renderButtons() {
    constexpr float button_width = 120.0f;
    ImGui::SetCursorPosX(((CONTENT_WIDTH - button_width) * 0.5f) + 20.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    if (ImGui::Button((std::string(ICON_FA_SAVE) + "  Save").c_str(), ImVec2(button_width, 40.0f))) {
        if (m_callbacks.onPomodoroDurationsApplied) {
            m_callbacks.onPomodoroDurationsApplied(m_state.temp_pomodoro_duration, m_state.temp_short_break_duration,
                                                   m_state.temp_long_break_duration);
        }
        if (m_callbacks.onWellnessSettingsApplied) {
            m_callbacks.onWellnessSettingsApplied(m_state.temp_water_interval, m_state.temp_water_daily_goal,
                                                  m_state.temp_standup_interval, m_state.temp_standup_duration,
                                                  m_state.temp_eye_interval, m_state.temp_eye_break_duration);
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
}

void SettingsPopup::renderDurationRow(const char* label, const char* minusId, const char* inputId, const char* plusId,
                                      int& value, int minValue, int maxValue) {
    constexpr float input_width = 60.0f;
    constexpr float button_size = 40.0f;

    ImGui::Text("%s", label);
    ImGui::Spacing();

    if (ImGui::Button((std::string(ICON_FA_MINUS) + minusId).c_str(), ImVec2(button_size, button_size))) {
        if (value > minValue) {
            value--;
        }
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(input_width);
    ImGui::InputInt(inputId, &value, 0, 0);
    ImGui::PopItemWidth();
    value = std::clamp(value, minValue, maxValue);

    ImGui::SameLine();
    if (ImGui::Button((std::string(ICON_FA_PLUS) + plusId).c_str(), ImVec2(button_size, button_size))) {
        if (value < maxValue) {
            value++;
        }
    }

    ImGui::Spacing();
}

} // namespace WorkBalance::App::UI::Components
