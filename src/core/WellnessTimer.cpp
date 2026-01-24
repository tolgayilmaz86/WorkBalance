#include <core/WellnessTimer.h>

#include <algorithm>

namespace WorkBalance::Core {

WellnessTimer::WellnessTimer(WellnessType type, int interval_seconds, int break_duration_seconds) noexcept
    : m_type(type), m_interval_seconds(interval_seconds), m_break_duration_seconds(break_duration_seconds),
      m_remaining_time(interval_seconds) {
}

bool WellnessTimer::update() noexcept {
    if (!m_running) {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_last_time);

    if (elapsed.count() >= 1) {
        m_remaining_time -= static_cast<int>(elapsed.count());
        m_last_time = now;

        if (m_remaining_time <= 0) {
            m_running = false;
            m_remaining_time = 0;

            if (m_in_break) {
                // Break completed
                m_in_break = false;
                m_remaining_time = m_interval_seconds;
                ++m_completed_count;
                return true;
            } else {
                // Interval completed, trigger reminder
                m_reminder_active = true;
                return true;
            }
        }
    }

    return false;
}

void WellnessTimer::start() noexcept {
    if (!m_running) {
        m_running = true;
        m_last_time = std::chrono::steady_clock::now();
    }
}

void WellnessTimer::pause() noexcept {
    m_running = false;
}

void WellnessTimer::toggle() noexcept {
    if (m_running) {
        pause();
    } else {
        start();
    }
}

void WellnessTimer::stop() noexcept {
    m_running = false;
    m_in_break = false;
    m_reminder_active = false;
    m_remaining_time = m_interval_seconds;
}

void WellnessTimer::reset() noexcept {
    m_running = false;
    m_in_break = false;
    m_reminder_active = false;
    m_remaining_time = m_interval_seconds;
}

void WellnessTimer::startBreak() noexcept {
    m_in_break = true;
    m_reminder_active = false;
    m_remaining_time = m_break_duration_seconds;
    m_running = true;
    m_last_time = std::chrono::steady_clock::now();
}

void WellnessTimer::endBreak() noexcept {
    m_in_break = false;
    m_reminder_active = false;
    m_remaining_time = m_interval_seconds;
    ++m_completed_count;
    start();
}

void WellnessTimer::acknowledgeReminder() noexcept {
    m_reminder_active = false;

    // For water reminders, just restart the timer
    // For standup/eye strain, user should start the break
    if (m_type == WellnessType::Water) {
        ++m_completed_count;
        reset();
        start();
    }
}

void WellnessTimer::setIntervalSeconds(int seconds) noexcept {
    m_interval_seconds = std::max(60, seconds); // Minimum 1 minute
    if (!m_in_break && !m_running) {
        m_remaining_time = m_interval_seconds;
    }
}

void WellnessTimer::setBreakDurationSeconds(int seconds) noexcept {
    m_break_duration_seconds = std::max(10, seconds); // Minimum 10 seconds
}

void WellnessTimer::resetDailyCounters() noexcept {
    m_completed_count = 0;
}

} // namespace WorkBalance::Core
