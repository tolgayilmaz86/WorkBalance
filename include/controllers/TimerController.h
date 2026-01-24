#pragma once

#include <core/Timer.h>
#include <system/IAudioService.h>

#include <functional>

namespace WorkBalance::Controllers {

/// @brief Controller for managing pomodoro timer operations with audio feedback
class TimerController {
  public:
    /// @brief Constructs a TimerController
    /// @param timer Reference to the timer to control
    /// @param audio Pointer to audio service for feedback sounds (can be nullptr)
    explicit TimerController(Core::Timer& timer, System::IAudioService* audio = nullptr);

    /// @brief Toggle timer running state (start/pause)
    void toggle();

    /// @brief Reset timer to initial duration for current mode
    void reset();

    /// @brief Set the timer mode (Pomodoro, ShortBreak, LongBreak)
    void setMode(Core::TimerMode mode);

    /// @brief Update timer state, should be called each frame
    /// @return true if timer completed this update
    [[nodiscard]] bool update();

    /// @brief Get the current timer
    [[nodiscard]] Core::Timer& getTimer() noexcept {
        return m_timer;
    }
    [[nodiscard]] const Core::Timer& getTimer() const noexcept {
        return m_timer;
    }

    /// @brief Apply new timer durations
    void applyDurations(int pomodoro_minutes, int short_break_minutes, int long_break_minutes);

    // Event callbacks - set these to be notified of timer events
    std::function<void(Core::TimerMode)> onModeChanged;
    std::function<void(int)> onTick;  // Called when remaining time changes (parameter: remaining seconds)
    std::function<void()> onComplete; // Called when timer completes

  private:
    void playClickSound();
    void playBellSound();

    Core::Timer& m_timer;
    System::IAudioService* m_audio;
    int m_last_remaining_time{0};
};

} // namespace WorkBalance::Controllers
