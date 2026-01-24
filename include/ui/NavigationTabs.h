#pragma once

#include <functional>
#include <imgui.h>
#include <ui/AppState.h>

namespace WorkBalance::UI {

/// @brief Callbacks for navigation tab interactions
struct NavigationCallbacks {
    std::function<void(NavigationTab)> onTabChanged;
};

/// @brief Renders navigation tabs as inline buttons matching the header button style
/// @details The tabs appear just above the timer frame inside the main window.
///          Each tab is a button with an icon and smooth hover/selection animations.
class NavigationTabs {
  public:
    /// @brief Constructs the navigation tabs component
    /// @param state Reference to app state for tracking active tab
    /// @param callbacks Callbacks for navigation events
    NavigationTabs(AppState& state, NavigationCallbacks callbacks);

    /// @brief Renders the navigation tabs (kept for compatibility, calls renderInline)
    /// @param viewport_pos Position of the viewport (unused)
    /// @param viewport_size Size of the viewport (unused)
    /// @param panel_height Height reserved for the tab panel (unused)
    void render(const ImVec2& viewport_pos, const ImVec2& viewport_size, float panel_height);

    /// @brief Renders the navigation tabs inline within the current window
    void renderInline();

    /// @brief Gets the height taken by the navigation tabs
    [[nodiscard]] static constexpr float getTabPanelHeight() noexcept {
        return TAB_SIZE + 16.0f; // Tab size plus spacing
    }

  private:
    void renderTabButton(NavigationTab tab, const char* icon, const char* tooltip, const ImVec4& active_color);

    AppState& m_state;
    NavigationCallbacks m_callbacks;

    // Tab styling constants (bigger than header buttons)
    static constexpr float TAB_SIZE = 48.0f;
    static constexpr float TAB_SPACING = 12.0f;
    static constexpr float TAB_ROUNDING = 8.0f;
};

} // namespace WorkBalance::UI
