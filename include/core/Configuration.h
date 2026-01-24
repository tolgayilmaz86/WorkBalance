#pragma once

#include <imgui.h>
#include <string_view>

namespace WorkBalance::Core {
struct Configuration {
    // Window settings
    static constexpr int DEFAULT_WINDOW_WIDTH = 500;
    static constexpr int DEFAULT_WINDOW_HEIGHT = 900;
    static constexpr std::string_view WINDOW_TITLE = "Work Balance";

    // OpenGL settings
#if defined(__APPLE__)
    static constexpr std::string_view GLSL_VERSION = "#version 150";
    static constexpr int GL_MAJOR_VERSION = 3;
    static constexpr int GL_MINOR_VERSION = 2;
    static constexpr bool USE_CORE_PROFILE = true;
#else
    static constexpr std::string_view GLSL_VERSION = "#version 130";
    static constexpr int GL_MAJOR_VERSION = 3;
    static constexpr int GL_MINOR_VERSION = 0;
    static constexpr bool USE_CORE_PROFILE = false;
#endif

    // Timer defaults (in seconds)
    static constexpr int DEFAULT_POMODORO_DURATION = 25 * 60;
    static constexpr int DEFAULT_SHORT_BREAK_DURATION = 5 * 60;
    static constexpr int DEFAULT_LONG_BREAK_DURATION = 15 * 60;

    // Timer defaults (in minutes) for UI/settings
    static constexpr int DEFAULT_POMODORO_MINUTES = 25;
    static constexpr int DEFAULT_SHORT_BREAK_MINUTES = 5;
    static constexpr int DEFAULT_LONG_BREAK_MINUTES = 15;

    // Wellness defaults (in minutes) for UI/settings
    static constexpr int DEFAULT_WATER_INTERVAL_MINUTES = 30;
    static constexpr int DEFAULT_WATER_DAILY_GOAL = 8;
    static constexpr int DEFAULT_STANDUP_INTERVAL_MINUTES = 45;
    static constexpr int DEFAULT_STANDUP_DURATION_MINUTES = 5;
    static constexpr int DEFAULT_EYE_INTERVAL_MINUTES = 20;
    static constexpr int DEFAULT_EYE_BREAK_DURATION_SECONDS = 20;

    // Window position defaults (-1 means use default/centered)
    static constexpr int DEFAULT_WINDOW_POSITION = -1;

    // Overlay position defaults
    static constexpr float DEFAULT_OVERLAY_POSITION_X = 100.0f;
    static constexpr float DEFAULT_OVERLAY_POSITION_Y = 100.0f;

    // Frame rate settings
    static constexpr double TARGET_FPS = 144.0;
    static constexpr double FRAME_TIME = 1.0 / TARGET_FPS;

    // Theme colors
    static constexpr ImVec4 POMODORO_BG_COLOR{0.85f, 0.35f, 0.35f, 1.0f};
    static constexpr ImVec4 SHORT_BREAK_BG_COLOR{0.22f, 0.52f, 0.54f, 1.0f};
    static constexpr ImVec4 LONG_BREAK_BG_COLOR{0.22f, 0.44f, 0.59f, 1.0f};

    // UI constants
    static constexpr float WINDOW_ROUNDING = 8.0f;
    static constexpr float FRAME_ROUNDING = 6.0f;
    static constexpr float BUTTON_ROUNDING = 8.0f;
    static constexpr float TIMER_FONT_SIZE = 120.0f;
    static constexpr float BUTTON_FONT_SIZE = 24.0f;
    static constexpr float OVERLAY_FONT_SIZE = 40.0f;
    static constexpr float REGULAR_FONT_SIZE = 18.0f;

    // Task defaults
    static constexpr int DEFAULT_ESTIMATED_POMODOROS = 1;
    static constexpr int DEFAULT_COMPLETED_POMODOROS = 0;
    static constexpr int MAX_TASK_NAME_LENGTH = 256;
};

} // namespace WorkBalance::Core
