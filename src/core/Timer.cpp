#include <core/Timer.h>

#include <utility>

namespace WorkBalance::Core {
namespace {

template <typename Fn>
void assignIfChanged(int& field, int value, Fn&& callback) noexcept {
    if (field == value) {
        return;
    }
    field = value;
    std::forward<Fn>(callback)();
}

[[nodiscard]] int durationForMode(TimerMode mode, int pomodoro, int short_break, int long_break) noexcept {
    switch (mode) {
        case TimerMode::Pomodoro:
            return pomodoro;
        case TimerMode::ShortBreak:
            return short_break;
        case TimerMode::LongBreak:
            return long_break;
    }
    return pomodoro;
}
} // namespace

Timer::Timer(int pomodoro_duration, int short_break_duration, int long_break_duration,
             std::shared_ptr<ITimeSource> time_source) noexcept
    : m_time_source(std::move(time_source)), m_pomodoro_duration(pomodoro_duration),
      m_short_break_duration(short_break_duration), m_long_break_duration(long_break_duration),
      m_remaining_time(pomodoro_duration), m_current_mode(TimerMode::Pomodoro), m_timer_state(TimerState::Stopped) {
}

bool Timer::update() noexcept {
    if (m_timer_state != TimerState::Running) {
        return false;
    }

    const auto current_time = m_time_source->now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - m_last_time);

    if (elapsed.count() <= 0) {
        return false;
    }

    m_remaining_time -= static_cast<int>(elapsed.count());
    m_last_time = current_time;

    if (m_remaining_time > 0) {
        return false;
    }

    m_remaining_time = 0;
    m_timer_state = TimerState::Stopped;
    return true;
}

void Timer::start() noexcept {
    if (m_timer_state == TimerState::Running) {
        return;
    }

    m_timer_state = TimerState::Running;
    m_last_time = m_time_source->now();
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
    m_remaining_time =
        durationForMode(m_current_mode, m_pomodoro_duration, m_short_break_duration, m_long_break_duration);
}

void Timer::setMode(TimerMode mode) noexcept {
    if (m_current_mode == mode) {
        reset();
        m_timer_state = TimerState::Stopped;
        return;
    }

    m_current_mode = mode;
    m_timer_state = TimerState::Stopped;
    reset();
}

void Timer::setPomodoroDuration(int seconds) noexcept {
    assignIfChanged(m_pomodoro_duration, seconds, [this] {
        if (m_current_mode == TimerMode::Pomodoro) {
            reset();
        }
    });
}

void Timer::setShortBreakDuration(int seconds) noexcept {
    assignIfChanged(m_short_break_duration, seconds, [this] {
        if (m_current_mode == TimerMode::ShortBreak) {
            reset();
        }
    });
}

void Timer::setLongBreakDuration(int seconds) noexcept {
    assignIfChanged(m_long_break_duration, seconds, [this] {
        if (m_current_mode == TimerMode::LongBreak) {
            reset();
        }
    });
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
