#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace WorkBalance::System {

/// @brief Callbacks for system tray menu actions
struct SystemTrayCallbacks {
    std::function<void()> onToggleTimer;
    std::function<void()> onToggleOverlayMode;
    std::function<void()> onShowWindow;
    std::function<void()> onQuit;
};

/// @brief Manages a system tray icon with context menu (Windows only)
class SystemTray {
  public:
    SystemTray();
    ~SystemTray();

    SystemTray(const SystemTray&) = delete;
    SystemTray& operator=(const SystemTray&) = delete;
    SystemTray(SystemTray&&) noexcept;
    SystemTray& operator=(SystemTray&&) noexcept;

    /// @brief Initializes the system tray icon
    /// @param callbacks The callbacks for menu actions
    /// @return true if initialization was successful
    [[nodiscard]] bool initialize(SystemTrayCallbacks callbacks);

    /// @brief Updates the tray icon tooltip text
    /// @param text The new tooltip text (e.g., timer status)
    void setTooltip(std::string_view text);

    /// @brief Updates the menu item text for timer state
    /// @param is_running Whether the timer is currently running
    void updateTimerState(bool is_running);

    /// @brief Updates the menu item text for window mode
    /// @param is_overlay Whether the window is in overlay mode
    void updateWindowMode(bool is_overlay);

    /// @brief Processes pending Windows messages for the tray
    /// Should be called in the main loop
    void processMessages();

    /// @brief Checks if the tray is initialized and active
    [[nodiscard]] bool isInitialized() const noexcept;

  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace WorkBalance::System
