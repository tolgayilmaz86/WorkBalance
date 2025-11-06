#include <core/Timer.h>

namespace WorkBalance::Core {

Timer::Timer(int pomodoro_duration, int short_break_duration, int long_break_duration) noexcept
    : m_pomodoro_duration(pomodoro_duration), m_short_break_duration(short_break_duration),
      m_long_break_duration(long_break_duration), m_remaining_time(pomodoro_duration),
      m_current_mode(TimerMode::Pomodoro), m_timer_state(TimerState::Stopped) {
}

bool Timer::update() noexcept {
    if (m_timer_state != TimerState::Running) {
        return false;
    }

    const auto current_time = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - m_last_time);

    if (elapsed.count() >= 1) {
        --m_remaining_time;
        m_last_time = current_time;

        if (m_remaining_time <= 0) {
            m_timer_state = TimerState::Stopped;
            return true;
        }
    }

    return false;
}

void Timer::start() noexcept {
    if (m_timer_state != TimerState::Running) {
        m_timer_state = TimerState::Running;
        m_last_time = std::chrono::steady_clock::now();
    }
}

void Timer::pause() noexcept {
    if (m_timer_state == TimerState::Running) {
        m_timer_state = TimerState::Paused;
    }
}

void Timer::toggle() noexcept {
    if (m_timer_state == TimerState::Running) {
        pause();
    } else {
        start();
    }
}

void Timer::stop() noexcept {
    m_timer_state = TimerState::Stopped;
}

void Timer::reset() noexcept {
    switch (m_current_mode) {
        case TimerMode::Pomodoro:
            m_remaining_time = m_pomodoro_duration;
            break;
        case TimerMode::ShortBreak:
            m_remaining_time = m_short_break_duration;
            break;
        case TimerMode::LongBreak:
            m_remaining_time = m_long_break_duration;
            break;
    }
}

void Timer::setMode(TimerMode mode) noexcept {
    m_timer_state = TimerState::Stopped;
    m_current_mode = mode;
    reset();
}

void Timer::setPomodoroDuration(int seconds) noexcept {
    m_pomodoro_duration = seconds;
}

void Timer::setShortBreakDuration(int seconds) noexcept {
    m_short_break_duration = seconds;
}

void Timer::setLongBreakDuration(int seconds) noexcept {
    m_long_break_duration = seconds;
}

int Timer::getPomodoroDuration() const noexcept {
    return m_pomodoro_duration;
}

int Timer::getShortBreakDuration() const noexcept {
    return m_short_break_duration;
}

int Timer::getLongBreakDuration() const noexcept {
    return m_long_break_duration;
}

} // namespace WorkBalance::Core
