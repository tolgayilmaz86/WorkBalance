#pragma once

#include <imgui.h>

namespace WorkBalance {

/// @brief Core UI visibility and navigation state
///
/// This focused struct contains only UI-related visibility flags and
/// display state. It follows the Single Responsibility Principle
/// by handling only UI display concerns.
///
/// @note Navigation tab state (active_tab) remains in AppState for now
/// to avoid circular dependencies with NavigationTab enum.
struct UIState {
    // Popup visibility
    bool show_settings = false;
    bool show_help = false;
    bool show_edit_task = false;
    bool show_add_task = false;
    bool show_timer_overlay = false;
    bool main_window_overlay_mode = false;

    // Tab menu state
    bool tab_menu_expanded = true; // For future collapse functionality

    // Current task selection
    int current_task_index = 0;

    // Background color (changes based on timer mode)
    ImVec4 background_color{0.76f, 0.35f, 0.35f, 1.0f}; // Pomodoro red

    // Overlay visibility settings (which timers to show in overlay mode)
    bool show_pomodoro_in_overlay = true;
    bool show_water_in_overlay = true;
    bool show_standup_in_overlay = true;
    bool show_eye_care_in_overlay = true;
};

} // namespace WorkBalance
