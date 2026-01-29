#pragma once

#include <string>

namespace WorkBalance::System {

/// @brief Utility class for managing Windows startup registration
class WindowsStartup {
  public:
    /// @brief Checks if the application is registered to start with Windows
    /// @return true if registered, false otherwise
    [[nodiscard]] static bool isRegistered();

    /// @brief Registers the application to start with Windows
    /// @return true on success, false on failure
    static bool registerStartup();

    /// @brief Unregisters the application from starting with Windows
    /// @return true on success, false on failure
    static bool unregisterStartup();

    /// @brief Sets the startup registration based on the desired state
    /// @param enabled true to register, false to unregister
    /// @return true on success, false on failure
    static bool setStartupEnabled(bool enabled);

  private:
    static constexpr const char* APP_NAME = "WorkBalance";
    static constexpr const char* STARTUP_KEY = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";

    /// @brief Gets the path to the current executable with startup flag
    [[nodiscard]] static std::string getStartupCommand();
};

} // namespace WorkBalance::System
