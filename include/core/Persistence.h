#pragma once

#include "Configuration.h"
#include "Task.h"

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace WorkBalance::Core {

/// @brief Error types that can occur during persistence operations
enum class PersistenceError { FileNotFound, FileOpenError, ParseError, WriteError, DirectoryCreateError };

/// @brief Get a human-readable description of a persistence error
[[nodiscard]] constexpr std::string_view getPersistenceErrorMessage(PersistenceError error) noexcept {
    switch (error) {
        case PersistenceError::FileNotFound:
            return "Configuration file not found";
        case PersistenceError::FileOpenError:
            return "Failed to open configuration file";
        case PersistenceError::ParseError:
            return "Failed to parse configuration file";
        case PersistenceError::WriteError:
            return "Failed to write configuration file";
        case PersistenceError::DirectoryCreateError:
            return "Failed to create configuration directory";
        default:
            return "Unknown persistence error";
    }
}

/// @brief User-configurable settings that persist across sessions
struct UserSettings {
    int pomodoro_duration_minutes = Configuration::DEFAULT_POMODORO_MINUTES;
    int short_break_duration_minutes = Configuration::DEFAULT_SHORT_BREAK_MINUTES;
    int long_break_duration_minutes = Configuration::DEFAULT_LONG_BREAK_MINUTES;
    bool auto_start_breaks = false;
    bool auto_start_pomodoros = false;
    // Pomodoro cycle settings
    int pomodoros_before_long_break = Configuration::DEFAULT_POMODOROS_BEFORE_LONG_BREAK;
    int long_breaks_in_cycle = Configuration::DEFAULT_LONG_BREAKS_IN_CYCLE;
    // Window positions
    float overlay_position_x = Configuration::DEFAULT_OVERLAY_POSITION_X;
    float overlay_position_y = Configuration::DEFAULT_OVERLAY_POSITION_Y;
    int main_window_x = Configuration::DEFAULT_WINDOW_POSITION; // -1 means use default (centered)
    int main_window_y = Configuration::DEFAULT_WINDOW_POSITION;
    // Overlay visibility settings
    bool show_pomodoro_in_overlay = true;
    bool show_water_in_overlay = true;
    bool show_standup_in_overlay = true;
    bool show_eye_care_in_overlay = true;
    // Wellness timer settings (in minutes)
    int water_interval_minutes = 30;
    int water_daily_goal = 8;
    int standup_interval_minutes = 45;
    int standup_duration_minutes = 5;
    int eye_care_interval_minutes = 20;
    int eye_care_break_seconds = 20;
    // Wellness auto-loop settings
    bool water_auto_loop = false;
    bool standup_auto_loop = false;
    bool eye_care_auto_loop = false;
    // Startup settings
    bool start_with_windows = false;
    bool start_minimized = true;
    // Sound settings
    bool pomodoro_sound_enabled = Configuration::DEFAULT_SOUND_ENABLED;
    int pomodoro_sound_volume = Configuration::DEFAULT_SOUND_VOLUME;
    bool water_sound_enabled = Configuration::DEFAULT_SOUND_ENABLED;
    int water_sound_volume = Configuration::DEFAULT_SOUND_VOLUME;
    bool standup_sound_enabled = Configuration::DEFAULT_SOUND_ENABLED;
    int standup_sound_volume = Configuration::DEFAULT_SOUND_VOLUME;
    bool eye_care_sound_enabled = Configuration::DEFAULT_SOUND_ENABLED;
    int eye_care_sound_volume = Configuration::DEFAULT_SOUND_VOLUME;
    // Notification settings
    bool pomodoro_notification_enabled = Configuration::DEFAULT_NOTIFICATIONS_ENABLED;
    bool water_notification_enabled = Configuration::DEFAULT_NOTIFICATIONS_ENABLED;
    bool standup_notification_enabled = Configuration::DEFAULT_NOTIFICATIONS_ENABLED;
    bool eye_care_notification_enabled = Configuration::DEFAULT_NOTIFICATIONS_ENABLED;
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
    /// @return std::expected with void on success, or PersistenceError on failure
    [[nodiscard]] std::expected<void, PersistenceError> save(const PersistentData& data) const;

    /// @brief Loads previously saved application state from disk
    /// @return std::expected with PersistentData on success, or PersistenceError on failure
    [[nodiscard]] std::expected<PersistentData, PersistenceError> load() const;

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
