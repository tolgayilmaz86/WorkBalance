#pragma once

#include "../core/Configuration.h"
#include "../core/Timer.h"
#include "../core/Task.h"
#include "../core/WellnessTypes.h"
#include "../system/IAudioService.h"
#include "../system/MainWindow.h"
#include <imgui.h>
#include <memory>
#include <string>
#include <format>
#include <array>
#include <utility>

namespace WorkBalance {

/// @brief Navigation tabs for the side menu
enum class NavigationTab {
    Pomodoro, ///< Focus timer with tasks
    Water,    ///< Hydration reminders
    Standup,  ///< Stand up and move reminders
    EyeCare   ///< Eye strain prevention (20-20-20 rule)
};

/// @brief Application State - Aggregation of domain models
///
/// This struct maintains backward compatibility with direct member access.
/// For new code, consider using the focused state structs in ui/state/:
/// - UIState.h: UI visibility and navigation state
/// - DragState.h: Window dragging state
/// - SettingsEditState.h: Temporary settings editing state
/// - TaskEditState.h: Task editing state
/// - RuntimeCounters.h: Runtime counters and statistics
struct AppState {
    // ===== UI Visibility State =====
    bool show_settings = false;
    bool show_help = false;
    bool show_edit_task = false;
    bool show_add_task = false;
    bool show_timer_overlay = false;
    bool main_window_overlay_mode = false;

    // ===== Navigation State =====
    NavigationTab active_tab = NavigationTab::Pomodoro;
    bool tab_menu_expanded = true; // For future collapse functionality

    // ===== Dragging States =====
    bool main_window_dragging = false;
    bool main_overlay_dragging = false;
    bool overlay_dragging = false;
    ImVec2 main_window_drag_offset{0.0f, 0.0f};
    ImVec2 main_overlay_drag_offset{0.0f, 0.0f};
    ImVec2 overlay_drag_offset{0.0f, 0.0f};
    ImVec2 overlay_position{Core::Configuration::DEFAULT_OVERLAY_POSITION_X,
                            Core::Configuration::DEFAULT_OVERLAY_POSITION_Y};

    // Main window position (-1 means use default/centered)
    int main_window_x = Core::Configuration::DEFAULT_WINDOW_POSITION;
    int main_window_y = Core::Configuration::DEFAULT_WINDOW_POSITION;

    // ===== Task Editing State =====
    int edit_task_index = -1;
    std::array<char, Core::Configuration::MAX_TASK_NAME_LENGTH> edit_task_name{};
    int edit_task_estimated_pomodoros = Core::Configuration::DEFAULT_ESTIMATED_POMODOROS;
    int edit_task_completed_pomodoros = Core::Configuration::DEFAULT_COMPLETED_POMODOROS;

    // ===== Settings Editing - Pomodoro =====
    int temp_pomodoro_duration = Core::Configuration::DEFAULT_POMODORO_MINUTES;
    int temp_short_break_duration = Core::Configuration::DEFAULT_SHORT_BREAK_MINUTES;
    int temp_long_break_duration = Core::Configuration::DEFAULT_LONG_BREAK_MINUTES;

    // ===== Settings Editing - Water =====
    int temp_water_interval = Core::Configuration::DEFAULT_WATER_INTERVAL_MINUTES;
    int temp_water_daily_goal = Core::Configuration::DEFAULT_WATER_DAILY_GOAL;

    // ===== Settings Editing - Standup =====
    int temp_standup_interval = Core::Configuration::DEFAULT_STANDUP_INTERVAL_MINUTES;
    int temp_standup_duration = Core::Configuration::DEFAULT_STANDUP_DURATION_MINUTES;

    // ===== Settings Editing - Eye Care =====
    int temp_eye_interval = Core::Configuration::DEFAULT_EYE_INTERVAL_MINUTES;
    int temp_eye_break_duration = Core::Configuration::DEFAULT_EYE_BREAK_DURATION_SECONDS;

    // ===== UI State =====
    int current_task_index = 0;
    ImVec4 background_color = Core::Configuration::POMODORO_BG_COLOR;

    // ===== Runtime Counters (cached from domain layer) =====
    int target_pomodoros = 0;
    int completed_pomodoros = 0;

    // ===== Wellness Counters (cached from domain layer) =====
    int water_glasses_consumed = 0;
    int water_daily_goal = Core::Configuration::DEFAULT_WATER_DAILY_GOAL;
    int standups_completed = 0;
    int eye_breaks_completed = 0;

    // ===== Overlay Visibility Settings =====
    bool show_pomodoro_in_overlay = true;
    bool show_water_in_overlay = true;
    bool show_standup_in_overlay = true;
    bool show_eye_care_in_overlay = true;

    // ===== Wellness Auto-Loop Settings =====
    bool water_auto_loop = false;
    bool standup_auto_loop = false;
    bool eye_care_auto_loop = false;

    // ===== Startup Settings =====
    bool start_minimized = true;
};

// Utility class for time formatting
class TimeFormatter {
  public:
    [[nodiscard]] static std::string formatTime(int seconds) {
        if (seconds < 0)
            seconds = 0;

        return std::format("{}:{:02}", seconds / 60, seconds % 60);
    }

    /// @brief Formats time in compact form (e.g., "45" for minutes only)
    [[nodiscard]] static std::string formatTimeCompact(int seconds) {
        if (seconds < 0)
            seconds = 0;

        int minutes = seconds / 60;
        return std::to_string(minutes);
    }

    /// @brief Formats timer display string with mode icon
    /// @param mode The current timer mode
    /// @param remaining_seconds The remaining time in seconds
    /// @return Formatted string with icon and time (e.g., "🕐  25:00")
    [[nodiscard]] static std::string formatTimerWithIcon(Core::TimerMode mode, int remaining_seconds) {
        const char* icon = nullptr;
        switch (mode) {
            case Core::TimerMode::Pomodoro:
                icon = "\xef\x80\x97"; // ICON_FA_CLOCK (U+f017)
                break;
            case Core::TimerMode::ShortBreak:
            case Core::TimerMode::LongBreak:
                icon = "\xef\x83\xb4"; // ICON_FA_COFFEE (U+f0f4)
                break;
        }
        return std::string(icon) + "  " + formatTime(remaining_seconds);
    }

    /// @brief Formats timer display string with mode icon in compact form (no seconds)
    /// @param mode The current timer mode
    /// @param remaining_seconds The remaining time in seconds
    /// @return Formatted string with icon and compact time (e.g., "🕐 25m")
    [[nodiscard]] static std::string formatTimerWithIconCompact(Core::TimerMode mode, int remaining_seconds) {
        const char* icon = nullptr;
        switch (mode) {
            case Core::TimerMode::Pomodoro:
                icon = "\xef\x80\x97"; // ICON_FA_CLOCK (U+f017)
                break;
            case Core::TimerMode::ShortBreak:
            case Core::TimerMode::LongBreak:
                icon = "\xef\x83\xb4"; // ICON_FA_COFFEE (U+f0f4)
                break;
        }
        return std::string(icon) + "  " + formatTimeCompact(remaining_seconds);
    }

    /// @brief Gets icon for wellness timer type
    [[nodiscard]] static const char* getWellnessIcon(Core::WellnessType type) {
        switch (type) {
            case Core::WellnessType::Water:
                return "\xef\x81\x83"; // ICON_FA_TINT (U+f043)
            case Core::WellnessType::Standup:
                return "\xef\x95\x94"; // ICON_FA_WALKING (U+f554)
            case Core::WellnessType::EyeStrain:
                return "\xef\x81\xae"; // ICON_FA_EYE (U+f06e)
            default:
                return "\xef\x80\x97"; // ICON_FA_CLOCK
        }
    }
};

// Theme manager for handling background colors using constexpr lookup
class ThemeManager {
  public:
    /// @brief Lookup table for timer mode to background color mapping
    static constexpr std::array<std::pair<Core::TimerMode, ImVec4>, 3> MODE_COLORS = {{
        {Core::TimerMode::Pomodoro, Core::Configuration::POMODORO_BG_COLOR},
        {Core::TimerMode::ShortBreak, Core::Configuration::SHORT_BREAK_BG_COLOR},
        {Core::TimerMode::LongBreak, Core::Configuration::LONG_BREAK_BG_COLOR},
    }};

    [[nodiscard]] static constexpr ImVec4 getBackgroundColor(Core::TimerMode mode) noexcept {
        for (const auto& [m, color] : MODE_COLORS) {
            if (m == mode) {
                return color;
            }
        }
        return Core::Configuration::POMODORO_BG_COLOR;
    }
};

} // namespace WorkBalance
