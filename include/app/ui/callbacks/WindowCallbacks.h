#pragma once

#include <ui/NavigationTabs.h>

#include <functional>

namespace WorkBalance::App::UI {

/// @brief Callbacks for window-related operations
struct WindowCallbacks {
    std::function<void()> onToggleOverlayMode;
    std::function<void()> onRequestClose;
    std::function<void(WorkBalance::NavigationTab)> onTabChanged;
};

} // namespace WorkBalance::App::UI
