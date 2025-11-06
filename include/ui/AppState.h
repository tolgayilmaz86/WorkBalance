#pragma once

#include "../core/Configuration.h"
#include "../core/Timer.h"
#include "../core/Task.h"
#include "../system/IAudioService.h"
#include "../system/MainWindow.h"
#include <imgui.h>
#include <memory>
#include <string>
#include <sstream>
#include <iomanip>

namespace WorkBalance {
// Application State - Aggregation of domain models
struct AppState {
    bool show_settings = false;
    bool show_help = false;
    bool show_edit_task = false;
    bool show_add_task = false;
    bool show_timer_overlay = false;
    bool main_window_overlay_mode = false;

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

    // Settings editing
    int temp_pomodoro_duration = 25;
    int temp_short_break_duration = 5;
    int temp_long_break_duration = 15;

    // UI state
    int current_task_index = 0;
    ImVec4 background_color = Core::Configuration::POMODORO_BG_COLOR;

    // Pomodoro counters cached from domain layer
    int target_pomodoros = 0;
    int completed_pomodoros = 0;
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
