#pragma once

#include <chrono>
#include <functional>

#include "WellnessTypes.h"

namespace WorkBalance::Core {

/// @brief A generic wellness timer that can be used for water, standup, or eye strain reminders
class WellnessTimer {
  public:
    using CompletionCallback = std::function<void(WellnessType)>;

    /// @brief Constructs a wellness timer with the specified type and interval
    /// @param type The wellness type this timer tracks
    /// @param interval_seconds The interval between reminders
    /// @param break_duration_seconds Optional break duration (for standup/eye strain)
    explicit WellnessTimer(WellnessType type, int interval_seconds, int break_duration_seconds = 0) noexcept;

    /// @brief Updates the timer state
    /// @return true if the timer completed (interval or break)
    [[nodiscard]] bool update() noexcept;

    /// @brief Starts or resumes the timer
    void start() noexcept;

    /// @brief Pauses the timer
    void pause() noexcept;

    /// @brief Toggles between running and paused states
    void toggle() noexcept;

    /// @brief Stops and resets the timer
    void stop() noexcept;

    /// @brief Resets the timer to the beginning of the interval
    void reset() noexcept;

    /// @brief Starts the break period (for standup/eye strain timers)
    void startBreak() noexcept;

    /// @brief Ends the break period and resets to interval
    void endBreak() noexcept;

    /// @brief Acknowledges a reminder and resets the timer
    void acknowledgeReminder() noexcept;

    // Getters
    [[nodiscard]] WellnessType getType() const noexcept {
        return m_type;
    }
    [[nodiscard]] int getRemainingTime() const noexcept {
        return m_remaining_time;
    }
    [[nodiscard]] int getIntervalSeconds() const noexcept {
        return m_interval_seconds;
    }
    [[nodiscard]] int getBreakDurationSeconds() const noexcept {
        return m_break_duration_seconds;
    }
    [[nodiscard]] bool isRunning() const noexcept {
        return m_running;
    }
    [[nodiscard]] bool isInBreak() const noexcept {
        return m_in_break;
    }
    [[nodiscard]] bool isReminderActive() const noexcept {
        return m_reminder_active;
    }
    [[nodiscard]] int getCompletedCount() const noexcept {
        return m_completed_count;
    }

    // Setters
    void setIntervalSeconds(int seconds) noexcept;
    void setBreakDurationSeconds(int seconds) noexcept;
    void setCompletedCount(int count) noexcept {
        m_completed_count = count;
    }
    void incrementCompleted() noexcept {
        ++m_completed_count;
    }

    /// @brief Resets daily counters (call at midnight or on user request)
    void resetDailyCounters() noexcept;

  private:
    WellnessType m_type;
    int m_interval_seconds;
    int m_break_duration_seconds;
    int m_remaining_time;
    int m_completed_count = 0;
    bool m_running = false;
    bool m_in_break = false;
    bool m_reminder_active = false;
    std::chrono::steady_clock::time_point m_last_time{};
};

} // namespace WorkBalance::Core
