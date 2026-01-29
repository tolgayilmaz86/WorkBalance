#pragma once

#include "INotificationService.h"

#include <memory>

namespace WorkBalance::System {

class NotificationManager final : public INotificationService {
  public:
    NotificationManager();
    ~NotificationManager() override;

    // Non-copyable, non-movable
    NotificationManager(const NotificationManager&) = delete;
    NotificationManager& operator=(const NotificationManager&) = delete;
    NotificationManager(NotificationManager&&) = delete;
    NotificationManager& operator=(NotificationManager&&) = delete;

    [[nodiscard]] bool initialize() override;
    [[nodiscard]] bool isSupported() const noexcept override;

    void showNotification(std::string_view title, std::string_view message) override;
    void showPomodoroComplete() override;
    void showShortBreakComplete() override;
    void showLongBreakComplete() override;
    void showWaterReminder() override;
    void showStandupReminder() override;
    void showEyeCareReminder() override;

  private:
    bool m_initialized = false;
    bool m_supported = false;
};

} // namespace WorkBalance::System
