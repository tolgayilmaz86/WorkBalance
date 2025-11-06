#pragma once

#include <chrono>
#include <functional>
#include <concepts>

namespace WorkBalance::Core {
enum class TimerMode { Pomodoro, ShortBreak, LongBreak };

enum class TimerState { Stopped, Running, Paused };

// Concept for timer callback
template <typename F>
concept TimerCallback = std::invocable<F>;

class Timer {
  public:
    Timer(int pomodoro_duration, int short_break_duration, int long_break_duration) noexcept;

    // Update timer state, returns true if timer completed
    [[nodiscard]] bool update() noexcept;

    void start() noexcept;

    void pause() noexcept;

    void toggle() noexcept;

    void stop() noexcept;

    void reset() noexcept;

    void setMode(TimerMode mode) noexcept;

    // Getters with [[nodiscard]]
    [[nodiscard]] int getRemainingTime() const noexcept {
        return m_remaining_time;
    }
    [[nodiscard]] TimerMode getCurrentMode() const noexcept {
        return m_current_mode;
    }
    [[nodiscard]] TimerState getState() const noexcept {
        return m_timer_state;
    }
    [[nodiscard]] bool isRunning() const noexcept {
        return m_timer_state == TimerState::Running;
    }
    [[nodiscard]] bool isPaused() const noexcept {
        return m_timer_state == TimerState::Paused;
    }
    [[nodiscard]] bool isStopped() const noexcept {
        return m_timer_state == TimerState::Stopped;
    }

    // Setters for duration configuration
    void setPomodoroDuration(int seconds) noexcept;
    void setShortBreakDuration(int seconds) noexcept;
    void setLongBreakDuration(int seconds) noexcept;

    [[nodiscard]] int getPomodoroDuration() const noexcept;
    [[nodiscard]] int getShortBreakDuration() const noexcept;
    [[nodiscard]] int getLongBreakDuration() const noexcept;

  private:
    int m_pomodoro_duration;
    int m_short_break_duration;
    int m_long_break_duration;
    int m_remaining_time;
    TimerMode m_current_mode;
    TimerState m_timer_state;
    std::chrono::steady_clock::time_point m_last_time{};
};

} // namespace WorkBalance::Core
