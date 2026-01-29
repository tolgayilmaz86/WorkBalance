#include <app/ui/WellnessViews.h>
#include <core/Configuration.h>
#include "assets/fonts/IconsFontAwesome5Pro.h"

#include <cmath>
#include <format>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace WorkBalance::App::UI {

namespace {
// Helper to format time as MM:SS
[[nodiscard]] std::string formatTime(int total_seconds) {
    if (total_seconds < 0)
        total_seconds = 0;
    return std::format("{}:{:02}", total_seconds / 60, total_seconds % 60);
}

// Helper to render centered text
void renderCenteredText(const char* text, ImFont* font = nullptr) {
    if (font)
        ImGui::PushFont(font);
    const float window_width = ImGui::GetWindowSize().x;
    const float text_width = ImGui::CalcTextSize(text).x;
    ImGui::SetCursorPosX((window_width - text_width) * 0.5f);
    ImGui::TextUnformatted(text);
    if (font)
        ImGui::PopFont();
}

// Helper to render a styled button matching the Pomodoro START/PAUSE button style
// Returns true if clicked
bool renderStyledButton(const char* label, const ImVec2& size, const ImVec4& accent_color, bool is_active,
                        ImFont* button_font = nullptr) {
    const float window_width = ImGui::GetWindowSize().x;
    ImGui::SetCursorPosX((window_width - size.x) * 0.5f);

    const ImVec2 button_pos = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float text_width = ImGui::CalcTextSize(label).x;
    float text_height = ImGui::CalcTextSize(label).y;
    if (button_font != nullptr) {
        ImGui::PushFont(button_font);
        text_width = ImGui::CalcTextSize(label).x;
        text_height = ImGui::CalcTextSize(label).y;
        ImGui::PopFont();
    }

    const ImU32 shadow_color = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
    const ImU32 highlight_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.4f));
    const ImU32 button_bg_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.95f));

    constexpr float rounding = 8.0f;
    constexpr float shadow_offset = 5.0f;

    if (is_active) {
        // Pressed/active state - inset look
        draw_list->AddRectFilled(ImVec2(button_pos.x - 1.0f, button_pos.y - 1.0f),
                                 ImVec2(button_pos.x + size.x + 1.0f, button_pos.y + size.y + 1.0f), shadow_color,
                                 rounding);
        draw_list->AddRectFilled(ImVec2(button_pos.x + 2.0f, button_pos.y + 2.0f),
                                 ImVec2(button_pos.x + size.x, button_pos.y + size.y), button_bg_color, rounding);
    } else {
        // Normal state - elevated with shadow
        draw_list->AddRectFilled(ImVec2(button_pos.x + shadow_offset, button_pos.y + shadow_offset),
                                 ImVec2(button_pos.x + size.x + shadow_offset, button_pos.y + size.y + shadow_offset),
                                 shadow_color, rounding);
        draw_list->AddRectFilled(button_pos, ImVec2(button_pos.x + size.x, button_pos.y + size.y), button_bg_color,
                                 rounding);
        draw_list->AddRect(button_pos, ImVec2(button_pos.x + size.x, button_pos.y + size.y), highlight_color, rounding,
                           0, 2.0f);
    }

    const bool clicked = ImGui::InvisibleButton("##StyledBtn", size);

    const ImVec2 text_pos = ImVec2(button_pos.x + ((size.x - text_width) * 0.5f) + (is_active ? 2.0f : 0.0f),
                                   button_pos.y + ((size.y - text_height) * 0.5f) + (is_active ? 2.0f : 0.0f));

    const ImU32 text_color = ImGui::ColorConvertFloat4ToU32(accent_color);

    if (button_font != nullptr) {
        draw_list->AddText(button_font, button_font->FontSize, text_pos, text_color, label);
    } else {
        draw_list->AddText(text_pos, text_color, label);
    }

    return clicked;
}

// Helper to render a secondary/smaller button (for less prominent actions)
bool renderSecondaryButton(const char* label, const ImVec2& size) {
    const float window_width = ImGui::GetWindowSize().x;
    ImGui::SetCursorPosX((window_width - size.x) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    const bool clicked = ImGui::Button(label, size);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    return clicked;
}

// Helper to render a text-only button (for skip/dismiss actions)
bool renderTextButton(const char* label, const ImVec2& size) {
    const float window_width = ImGui::GetWindowSize().x;
    ImGui::SetCursorPosX((window_width - size.x) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));

    const bool clicked = ImGui::Button(label, size);

    ImGui::PopStyleColor(2);

    return clicked;
}

// Helper to begin a centered timer frame (matching Pomodoro style)
bool beginTimerFrame(const char* id, float frame_height = 280.0f) {
    const float window_width = ImGui::GetWindowSize().x;
    const float frame_width = std::min(600.0f, window_width - 40.0f);
    constexpr float frame_padding = 5.0f;

    ImGui::SetCursorPosX((window_width - frame_width) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 0.05f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(frame_padding, frame_padding));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.5f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));

    constexpr ImGuiWindowFlags timer_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    return ImGui::BeginChild(id, ImVec2(frame_width, frame_height), 1, timer_flags);
}

// Helper to end a timer frame
void endTimerFrame() {
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
    ImGui::Spacing();
}

// Helper to render a circular progress indicator
void renderCircularProgress(float progress, float radius, const ImVec4& color, const ImVec4& bg_color) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec2 center = ImGui::GetCursorScreenPos();
    const ImVec2 actual_center(center.x + radius, center.y + radius);

    // Background circle
    draw_list->AddCircle(actual_center, radius, ImGui::ColorConvertFloat4ToU32(bg_color), 64, 4.0f);

    // Progress arc
    if (progress > 0.0f) {
        const float start_angle = -static_cast<float>(M_PI) * 0.5f;
        const float end_angle = start_angle + (2.0f * static_cast<float>(M_PI) * progress);
        draw_list->PathArcTo(actual_center, radius, start_angle, end_angle, 64);
        draw_list->PathStroke(ImGui::ColorConvertFloat4ToU32(color), 0, 6.0f);
    }

    // Reserve space
    ImGui::Dummy(ImVec2(radius * 2.0f, radius * 2.0f));
}

} // anonymous namespace

// ============================================================================
// WaterReminderView Implementation
// ============================================================================

WaterReminderView::WaterReminderView(App::ImGuiLayer& imgui, Core::WellnessTimer& timer, AppState& state,
                                     WellnessViewCallbacks callbacks)
    : m_imgui(imgui), m_timer(timer), m_state(state), m_callbacks(std::move(callbacks)) {
}

void WaterReminderView::render() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ImGui::Spacing();

    // Timer frame - only timer and button
    if (beginTimerFrame("WaterTimerFrame", 240.0f)) {
        ImGui::Spacing();
        renderTimer();
        ImGui::Spacing();
        ImGui::Spacing();
        renderControls();
    }
    endTimerFrame();

    // Auto-loop checkbox below the frame
    ImGui::Spacing();
    {
        const float window_width = ImGui::GetWindowSize().x;
        const float checkbox_width = 130.0f;
        ImGui::SetCursorPosX((window_width - checkbox_width) * 0.5f);
        ImGui::Checkbox("Auto restart", &m_state.water_auto_loop);
    }

    // Goal tracker below the frame
    ImGui::Spacing();
    renderGoalTracker();

    ImGui::PopStyleColor();
}

void WaterReminderView::renderTimer() {
    const float window_width = ImGui::GetWindowSize().x;
    const std::string time_str = formatTime(m_timer.getRemainingTime());

    // Large timer display
    ImFont* timer_font = m_imgui.timerFont();
    if (timer_font)
        ImGui::PushFont(timer_font);

    const float time_width = ImGui::CalcTextSize(time_str.c_str()).x;
    ImGui::SetCursorPosX((window_width - time_width) * 0.5f);

    if (m_timer.isReminderActive()) {
        // Pulsing effect when reminder is active
        const float pulse = (std::sin(static_cast<float>(ImGui::GetTime()) * 4.0f) + 1.0f) * 0.5f;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f - pulse * 0.3f, 1.0f - pulse * 0.3f, 1.0f));
    }

    ImGui::TextUnformatted(time_str.c_str());

    if (m_timer.isReminderActive()) {
        ImGui::PopStyleColor();
    }

    if (timer_font)
        ImGui::PopFont();
}

void WaterReminderView::renderGoalTracker() {
    const float window_width = ImGui::GetWindowSize().x;
    const int completed = m_timer.getCompletedCount();
    const int goal = m_state.water_daily_goal;

    // Goal text
    const std::string goal_text = std::format("{} / {} glasses today", completed, goal);
    renderCenteredText(goal_text.c_str());

    ImGui::Spacing();

    // Visual glass indicators
    constexpr float glass_size = 28.0f;
    constexpr float glass_spacing = 6.0f;
    const float total_width = (glass_size * goal) + (glass_spacing * (goal - 1));
    ImGui::SetCursorPosX((window_width - total_width) * 0.5f);

    for (int i = 0; i < goal; ++i) {
        if (i > 0)
            ImGui::SameLine(0.0f, glass_spacing);

        const bool is_filled = (i < completed);
        ImGui::PushStyleColor(ImGuiCol_Text,
                              is_filled ? ImVec4(0.4f, 0.8f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
        ImGui::Text("%s", is_filled ? ICON_FA_TINT : ICON_FA_TINT_SLASH);
        ImGui::PopStyleColor();
    }
}

void WaterReminderView::renderControls() {
    constexpr ImVec2 button_size(240.0f, 60.0f);
    const ImVec4 water_color = Core::WellnessDefaults::WATER_BG_COLOR;

    if (m_timer.isReminderActive()) {
        // Show acknowledge button to restart timer
        if (renderStyledButton("START", button_size, water_color, false, m_imgui.buttonFont())) {
            if (m_callbacks.onAcknowledge) {
                m_callbacks.onAcknowledge();
            }
        }
    } else {
        // Show start/pause button
        const bool is_running = m_timer.isRunning();
        const char* label = is_running ? "PAUSE" : "START";
        if (renderStyledButton(label, button_size, water_color, is_running, m_imgui.buttonFont())) {
            if (m_callbacks.onToggleTimer) {
                m_callbacks.onToggleTimer();
            }
        }
    }

    ImGui::Spacing();

    // Reset daily counter button (smaller, less prominent)
    if (renderSecondaryButton(ICON_FA_REDO "  Reset Day", ImVec2(120.0f, 30.0f))) {
        if (m_callbacks.onResetDaily) {
            m_callbacks.onResetDaily();
        }
    }
}

// ============================================================================
// StandupReminderView Implementation
// ============================================================================

StandupReminderView::StandupReminderView(App::ImGuiLayer& imgui, Core::WellnessTimer& timer, AppState& state,
                                         WellnessViewCallbacks callbacks)
    : m_imgui(imgui), m_timer(timer), m_state(state), m_callbacks(std::move(callbacks)) {
}

void StandupReminderView::render() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ImGui::Spacing();

    // Timer frame - only timer and button
    if (beginTimerFrame("StandupTimerFrame", 240.0f)) {
        ImGui::Spacing();
        if (m_timer.isInBreak()) {
            renderBreakMode();
        } else {
            renderTimer();
        }
        ImGui::Spacing();
        ImGui::Spacing();
        renderControls();
    }
    endTimerFrame();

    // Auto-loop checkbox below the frame
    ImGui::Spacing();
    {
        const float window_width = ImGui::GetWindowSize().x;
        const float checkbox_width = 130.0f;
        ImGui::SetCursorPosX((window_width - checkbox_width) * 0.5f);
        ImGui::Checkbox("Auto-restart", &m_state.standup_auto_loop);
    }

    // Stats below the frame
    ImGui::Spacing();
    renderStats();

    ImGui::PopStyleColor();
}

void StandupReminderView::renderTimer() {
    const float window_width = ImGui::GetWindowSize().x;
    const std::string time_str = formatTime(m_timer.getRemainingTime());

    ImFont* timer_font = m_imgui.timerFont();
    if (timer_font)
        ImGui::PushFont(timer_font);

    const float time_width = ImGui::CalcTextSize(time_str.c_str()).x;
    ImGui::SetCursorPosX((window_width - time_width) * 0.5f);

    if (m_timer.isReminderActive()) {
        const float pulse = (std::sin(static_cast<float>(ImGui::GetTime()) * 4.0f) + 1.0f) * 0.5f;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f - pulse * 0.3f, 1.0f - pulse * 0.5f, 1.0f));
    }

    ImGui::TextUnformatted(time_str.c_str());

    if (m_timer.isReminderActive()) {
        ImGui::PopStyleColor();
    }

    if (timer_font)
        ImGui::PopFont();
}

void StandupReminderView::renderBreakMode() {
    const float window_width = ImGui::GetWindowSize().x;
    const std::string time_str = formatTime(m_timer.getRemainingTime());

    // Animated standing icon
    ImGui::Spacing();
    const float bounce = std::abs(std::sin(static_cast<float>(ImGui::GetTime()) * 2.0f)) * 5.0f;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - bounce);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.7f, 1.0f, 1.0f));
    renderCenteredText(ICON_FA_WALKING);
    ImGui::PopStyleColor();

    ImGui::Spacing();

    ImFont* timer_font = m_imgui.timerFont();
    if (timer_font)
        ImGui::PushFont(timer_font);

    const float time_width = ImGui::CalcTextSize(time_str.c_str()).x;
    ImGui::SetCursorPosX((window_width - time_width) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.7f, 1.0f, 1.0f));
    ImGui::TextUnformatted(time_str.c_str());
    ImGui::PopStyleColor();

    if (timer_font)
        ImGui::PopFont();

    ImGui::Spacing();
    renderCenteredText("Stay standing and stretch!");
}

void StandupReminderView::renderStats() {
    const std::string stats_text = std::format("{} stand-up breaks today", m_timer.getCompletedCount());
    renderCenteredText(stats_text.c_str());
}

void StandupReminderView::renderControls() {
    constexpr ImVec2 button_size(240.0f, 60.0f);
    const ImVec4 standup_color = Core::WellnessDefaults::STANDUP_BG_COLOR;

    if (m_timer.isInBreak()) {
        // Show "Done" button during break
        if (renderStyledButton("DONE STANDING", button_size, standup_color, false, m_imgui.buttonFont())) {
            if (m_callbacks.onEndBreak) {
                m_callbacks.onEndBreak();
            }
        }
    } else if (m_timer.isReminderActive()) {
        // Show "Start Break" button
        if (renderStyledButton("START BREAK", button_size, standup_color, false, m_imgui.buttonFont())) {
            if (m_callbacks.onStartBreak) {
                m_callbacks.onStartBreak();
            }
        }

        ImGui::Spacing();

        // Skip option
        if (renderTextButton("Skip this one", ImVec2(100.0f, 25.0f))) {
            if (m_callbacks.onAcknowledge) {
                m_callbacks.onAcknowledge();
            }
        }
    } else {
        // Show start/pause button
        const bool is_running = m_timer.isRunning();
        const char* label = is_running ? "PAUSE" : "START";
        if (renderStyledButton(label, button_size, standup_color, is_running, m_imgui.buttonFont())) {
            if (m_callbacks.onToggleTimer) {
                m_callbacks.onToggleTimer();
            }
        }
    }
}

// ============================================================================
// EyeCareReminderView Implementation
// ============================================================================

EyeCareReminderView::EyeCareReminderView(App::ImGuiLayer& imgui, Core::WellnessTimer& timer, AppState& state,
                                         WellnessViewCallbacks callbacks)
    : m_imgui(imgui), m_timer(timer), m_state(state), m_callbacks(std::move(callbacks)) {
}

void EyeCareReminderView::render() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ImGui::Spacing();

    // Timer frame - only timer and button
    if (beginTimerFrame("EyeCareTimerFrame", 240.0f)) {
        ImGui::Spacing();
        if (m_timer.isInBreak()) {
            renderBreakMode();
        } else {
            renderTimer();
        }
        ImGui::Spacing();
        ImGui::Spacing();
        renderControls();
    }
    endTimerFrame();

    // Auto-loop checkbox below the frame
    ImGui::Spacing();
    {
        const float window_width = ImGui::GetWindowSize().x;
        const float checkbox_width = 130.0f;
        ImGui::SetCursorPosX((window_width - checkbox_width) * 0.5f);
        ImGui::Checkbox("Auto-restart", &m_state.eye_care_auto_loop);
    }

    // Stats and tips below the frame
    ImGui::Spacing();
    renderStats();
    ImGui::Spacing();
    renderTip();

    ImGui::PopStyleColor();
}

void EyeCareReminderView::renderTimer() {
    const float window_width = ImGui::GetWindowSize().x;
    const std::string time_str = formatTime(m_timer.getRemainingTime());

    ImFont* timer_font = m_imgui.timerFont();
    if (timer_font)
        ImGui::PushFont(timer_font);

    const float time_width = ImGui::CalcTextSize(time_str.c_str()).x;
    ImGui::SetCursorPosX((window_width - time_width) * 0.5f);

    if (m_timer.isReminderActive()) {
        const float pulse = (std::sin(static_cast<float>(ImGui::GetTime()) * 4.0f) + 1.0f) * 0.5f;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f + pulse * 0.5f, 1.0f, 0.8f + pulse * 0.2f, 1.0f));
    }

    ImGui::TextUnformatted(time_str.c_str());

    if (m_timer.isReminderActive()) {
        ImGui::PopStyleColor();
    }

    if (timer_font)
        ImGui::PopFont();
}

void EyeCareReminderView::renderBreakMode() {
    const float window_width = ImGui::GetWindowSize().x;
    const std::string time_str = formatTime(m_timer.getRemainingTime());

    // Animated eye icon (blinking effect)
    ImGui::Spacing();
    const float blink_cycle = std::fmod(static_cast<float>(ImGui::GetTime()), 3.0f);
    const bool is_blinking = (blink_cycle > 2.8f);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 1.0f, 0.8f, 1.0f));
    renderCenteredText(is_blinking ? ICON_FA_EYE_SLASH : ICON_FA_EYE);
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // Large countdown for the 20 seconds
    ImFont* timer_font = m_imgui.timerFont();
    if (timer_font)
        ImGui::PushFont(timer_font);

    const float time_width = ImGui::CalcTextSize(time_str.c_str()).x;
    ImGui::SetCursorPosX((window_width - time_width) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 1.0f, 0.8f, 1.0f));
    ImGui::TextUnformatted(time_str.c_str());
    ImGui::PopStyleColor();

    if (timer_font)
        ImGui::PopFont();

    ImGui::Spacing();
    renderCenteredText("Look at something 20 feet away");
}

void EyeCareReminderView::renderStats() {
    const std::string stats_text = std::format("{} eye breaks today", m_timer.getCompletedCount());
    renderCenteredText(stats_text.c_str());
}

void EyeCareReminderView::renderTip() {
    // Rotating tips
    static const char* tips[] = {"Blink frequently to keep eyes moist",
                                 "Adjust screen brightness to match surroundings",
                                 "Position screen at arm's length away", "Use artificial tears if eyes feel dry",
                                 "Take regular breaks from screen time"};

    const int tip_index = static_cast<int>(ImGui::GetTime() / 10.0) % 5;

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
    ImGui::Spacing();
    renderCenteredText(ICON_FA_LIGHTBULB);
    renderCenteredText(tips[tip_index]);
    ImGui::PopStyleColor();
}

void EyeCareReminderView::renderControls() {
    constexpr ImVec2 button_size(240.0f, 60.0f);
    const ImVec4 eye_color = Core::WellnessDefaults::EYE_STRAIN_BG_COLOR;

    if (m_timer.isInBreak()) {
        // Show progress - break will auto-complete
        const float window_width = ImGui::GetWindowSize().x;
        ImGui::SetCursorPosX((window_width - 160.0f) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
        ImGui::Text("Relax and look away...");
        ImGui::PopStyleColor();

        ImGui::Spacing();

        // Option to end early
        if (renderStyledButton("DONE", ImVec2(160.0f, 50.0f), eye_color, false, m_imgui.buttonFont())) {
            if (m_callbacks.onEndBreak) {
                m_callbacks.onEndBreak();
            }
        }
    } else if (m_timer.isReminderActive()) {
        // Show "Start Break" button
        if (renderStyledButton("START", button_size, eye_color, false, m_imgui.buttonFont())) {
            if (m_callbacks.onStartBreak) {
                m_callbacks.onStartBreak();
            }
        }

        ImGui::Spacing();

        // Skip option
        if (renderTextButton("Skip this one", ImVec2(100.0f, 25.0f))) {
            if (m_callbacks.onAcknowledge) {
                m_callbacks.onAcknowledge();
            }
        }
    } else {
        // Show start/pause button
        const bool is_running = m_timer.isRunning();
        const char* label = is_running ? "PAUSE" : "START";
        if (renderStyledButton(label, button_size, eye_color, is_running, m_imgui.buttonFont())) {
            if (m_callbacks.onToggleTimer) {
                m_callbacks.onToggleTimer();
            }
        }
    }
}

} // namespace WorkBalance::App::UI
