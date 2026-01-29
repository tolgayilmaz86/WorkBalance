#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace WorkBalance::System {

/// @brief Interface for system notification services
class INotificationService {
  public:
    virtual ~INotificationService() = default;

    /// @brief Initialize the notification service
    /// @return true if initialization succeeded
    [[nodiscard]] virtual bool initialize() = 0;

    /// @brief Check if notifications are supported on this system
    [[nodiscard]] virtual bool isSupported() const noexcept = 0;

    /// @brief Show a notification with title and message
    /// @param title The notification title
    /// @param message The notification body text
    virtual void showNotification(std::string_view title, std::string_view message) = 0;

    /// @brief Show a pomodoro completion notification
    virtual void showPomodoroComplete() = 0;

    /// @brief Show a short break completion notification
    virtual void showShortBreakComplete() = 0;

    /// @brief Show a long break completion notification
    virtual void showLongBreakComplete() = 0;

    /// @brief Show a water reminder notification
    virtual void showWaterReminder() = 0;

    /// @brief Show a standup reminder notification
    virtual void showStandupReminder() = 0;

    /// @brief Show an eye care reminder notification
    virtual void showEyeCareReminder() = 0;
};

/// @brief Create the platform-specific notification service
[[nodiscard]] std::unique_ptr<INotificationService> createNotificationService();

} // namespace WorkBalance::System
