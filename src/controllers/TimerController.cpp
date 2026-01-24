#include <controllers/TimerController.h>

namespace WorkBalance::Controllers {

namespace {
constexpr int SECONDS_PER_MINUTE = 60;
} // namespace

TimerController::TimerController(Core::Timer& timer, System::IAudioService* audio)
    : m_timer(timer), m_audio(audio), m_last_remaining_time(timer.getRemainingTime()) {
}

void TimerController::toggle() {
    playClickSound();
    m_timer.toggle();
}

void TimerController::reset() {
    m_timer.reset();
    m_last_remaining_time = m_timer.getRemainingTime();
    if (onTick) {
        onTick(m_last_remaining_time);
    }
}

void TimerController::setMode(Core::TimerMode mode) {
    m_timer.setMode(mode);
    reset();
    if (onModeChanged) {
        onModeChanged(mode);
    }
}

bool TimerController::update() {
    const int current_remaining = m_timer.getRemainingTime();

    // Check for tick (time changed)
    if (current_remaining != m_last_remaining_time) {
        m_last_remaining_time = current_remaining;
        if (onTick) {
            onTick(current_remaining);
        }
    }

    // Update timer and check for completion
    if (m_timer.update()) {
        // Timer just completed
        playBellSound();
        if (onComplete) {
            onComplete();
        }
        return true;
    }

    // Also check if timer reached zero while running
    if (current_remaining <= 0 && m_timer.isRunning()) {
        m_timer.stop();
        playBellSound();
        if (onComplete) {
            onComplete();
        }
        return true;
    }

    return false;
}

void TimerController::applyDurations(int pomodoro_minutes, int short_break_minutes, int long_break_minutes) {
    m_timer.setPomodoroDuration(pomodoro_minutes * SECONDS_PER_MINUTE);
    m_timer.setShortBreakDuration(short_break_minutes * SECONDS_PER_MINUTE);
    m_timer.setLongBreakDuration(long_break_minutes * SECONDS_PER_MINUTE);

    if (m_timer.getState() == Core::TimerState::Stopped) {
        reset();
    }
}

void TimerController::playClickSound() {
    if (m_audio && m_audio->isInitialized()) {
        m_audio->playClickSound();
    }
}

void TimerController::playBellSound() {
    if (m_audio && m_audio->isInitialized()) {
        m_audio->playBellSound();
    }
}

} // namespace WorkBalance::Controllers
