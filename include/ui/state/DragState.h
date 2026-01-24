#pragma once

#include "../../core/Configuration.h"
#include <imgui.h>

namespace WorkBalance {

/// @brief State for tracking window/element dragging
struct DragState {
    bool is_dragging = false;
    ImVec2 drag_offset{0.0f, 0.0f};
};

/// @brief Aggregated drag states for all draggable windows
struct WindowDragStates {
    DragState main_window;
    DragState main_overlay;
    DragState overlay;

    // Overlay position (persisted)
    ImVec2 overlay_position{Core::Configuration::DEFAULT_OVERLAY_POSITION_X,
                            Core::Configuration::DEFAULT_OVERLAY_POSITION_Y};

    // Main window position (-1 means use default/centered)
    int main_window_x = Core::Configuration::DEFAULT_WINDOW_POSITION;
    int main_window_y = Core::Configuration::DEFAULT_WINDOW_POSITION;
};

} // namespace WorkBalance
