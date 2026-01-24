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
#include <sstream>
#include <iomanip>

namespace WorkBalance {

/// @brief Navigation tabs for the side menu
enum class NavigationTab {
    Pomodoro, ///< Focus timer with tasks
    Water,    ///< Hydration reminders
    Standup,  ///< Stand up and move reminders
    EyeCare   ///< Eye strain prevention (20-20-20 rule)
};

// Application State - Aggregation of domain models
struct AppState {
    bool show_settings = false;
    bool show_help = false;
    bool show_edit_task = false;
    bool show_add_task = false;
    bool show_timer_overlay = false;
    bool main_window_overlay_mode = false;

    // Navigation state
    NavigationTab active_tab = NavigationTab::Pomodoro;
    bool tab_menu_expanded = true; // For future collapse functionality

    // Dragging states
    bool main_window_dragging = false;
    bool main_overlay_dragging = false;
    bool overlay_dragging = false;
    ImVec2 main_window_drag_offset{0.0f, 0.0f};
    ImVec2 main_overlay_drag_offset{0.0f, 0.0f};
    ImVec2 overlay_drag_offset{0.0f, 0.0f};
    ImVec2 overlay_position{100.0f, 100.0f};

    // Task editing state
    int edit_task_index = -1;
    char edit_task_name[256] = "";
    int edit_task_estimated_pomodoros = 1;
    int edit_task_completed_pomodoros = 0;

    // Settings editing - Pomodoro
    int temp_pomodoro_duration = 25;
    int temp_short_break_duration = 5;
    int temp_long_break_duration = 15;

    // Settings editing - Water
    int temp_water_interval = 30;  // minutes
    int temp_water_daily_goal = 8; // glasses

    // Settings editing - Standup
    int temp_standup_interval = 45; // minutes
    int temp_standup_duration = 5;  // minutes

    // Settings editing - Eye Care
    int temp_eye_interval = 20;       // minutes
    int temp_eye_break_duration = 20; // seconds

    // UI state
    int current_task_index = 0;
    ImVec4 background_color = Core::Configuration::POMODORO_BG_COLOR;

    // Pomodoro counters cached from domain layer
    int target_pomodoros = 0;
    int completed_pomodoros = 0;

    // Wellness counters cached from domain layer
    int water_glasses_consumed = 0;
    int water_daily_goal = 8;
    int standups_completed = 0;
    int eye_breaks_completed = 0;
};

// Utility class for time formatting
class TimeFormatter {
  public:
    [[nodiscard]] static std::string formatTime(int seconds) {
        if (seconds < 0)
            seconds = 0;

        int minutes = seconds / 60;
        int secs = seconds % 60;

        std::stringstream ss;
        ss << minutes << ":" << std::setfill('0') << std::setw(2) << secs;
        std::string result = ss.str();

        // Fallback if colon is missing (locale issues)
        if (result.find(':') == std::string::npos) {
            result = std::to_string(minutes) + ":" + (secs < 10 ? "0" : "") + std::to_string(secs);
        }

        return result;
    }
};

// Theme manager for handling background colors
class ThemeManager {
  public:
    [[nodiscard]] static ImVec4 getBackgroundColor(Core::TimerMode mode) noexcept {
        using namespace Core;
        switch (mode) {
            case TimerMode::Pomodoro:
                return Configuration::POMODORO_BG_COLOR;
            case TimerMode::ShortBreak:
                return Configuration::SHORT_BREAK_BG_COLOR;
            case TimerMode::LongBreak:
                return Configuration::LONG_BREAK_BG_COLOR;
            default:
                return Configuration::POMODORO_BG_COLOR;
        }
    }
};

} // namespace WorkBalance
