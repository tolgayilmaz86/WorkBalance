#pragma once

#include "Task.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace WorkBalance::Core {

/// @brief User-configurable settings that persist across sessions
struct UserSettings {
    int pomodoro_duration_minutes = 25;
    int short_break_duration_minutes = 5;
    int long_break_duration_minutes = 15;
    bool auto_start_breaks = false;
    bool auto_start_pomodoros = false;
    float overlay_position_x = 100.0f;
    float overlay_position_y = 100.0f;
};

/// @brief Persistent application data including tasks and settings
struct PersistentData {
    UserSettings settings;
    std::vector<Task> tasks;
    int current_task_index = 0;
};

/// @brief Handles saving and loading application state to/from disk
class PersistenceManager {
  public:
    PersistenceManager();
    explicit PersistenceManager(std::filesystem::path config_directory);

    /// @brief Saves the current application state to disk
    /// @param data The data to persist
    /// @return true if save was successful
    [[nodiscard]] bool save(const PersistentData& data) const;

    /// @brief Loads previously saved application state from disk
    /// @return The loaded data, or std::nullopt if no data exists or load failed
    [[nodiscard]] std::optional<PersistentData> load() const;

    /// @brief Checks if a saved state file exists
    [[nodiscard]] bool hasSavedData() const;

    /// @brief Gets the path to the configuration file
    [[nodiscard]] const std::filesystem::path& getConfigPath() const noexcept {
        return m_config_path;
    }

  private:
    [[nodiscard]] static std::filesystem::path getDefaultConfigDirectory();
    [[nodiscard]] static std::string serializeToJson(const PersistentData& data);
    [[nodiscard]] static std::optional<PersistentData> deserializeFromJson(const std::string& json);

    std::filesystem::path m_config_path;
};

} // namespace WorkBalance::Core
