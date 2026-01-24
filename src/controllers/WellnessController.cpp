#include <controllers/WellnessController.h>

namespace WorkBalance::Controllers {

namespace {
constexpr int SECONDS_PER_MINUTE = 60;
} // namespace

WellnessController::WellnessController(std::unique_ptr<Core::WellnessTimer> water,
                                       std::unique_ptr<Core::WellnessTimer> standup,
                                       std::unique_ptr<Core::WellnessTimer> eye_care, System::IAudioService* audio)
    : m_water_timer(std::move(water)), m_standup_timer(std::move(standup)), m_eye_care_timer(std::move(eye_care)),
      m_audio(audio) {
}

void WellnessController::update() {
    if (m_water_timer && m_water_timer->update()) {
        handleTimerComplete(Core::WellnessType::Water);
    }
    if (m_standup_timer && m_standup_timer->update()) {
        handleTimerComplete(Core::WellnessType::Standup);
    }
    if (m_eye_care_timer && m_eye_care_timer->update()) {
        handleTimerComplete(Core::WellnessType::EyeStrain);
    }
}

void WellnessController::handleTimerComplete(Core::WellnessType type) {
    playBellSound();

    // For timers with breaks, the completion is handled differently
    switch (type) {
        case Core::WellnessType::Water:
            // Water reminder - just notify user
            break;
        case Core::WellnessType::Standup:
            // If break completed, restart interval timer
            if (m_standup_timer && !m_standup_timer->isInBreak()) {
                m_standup_timer->start();
            }
            break;
        case Core::WellnessType::EyeStrain:
            // If break completed, restart interval timer
            if (m_eye_care_timer && !m_eye_care_timer->isInBreak()) {
                m_eye_care_timer->start();
            }
            break;
        default:
            break;
    }

    if (onTimerComplete) {
        onTimerComplete(type);
    }
}

// Water timer controls
void WellnessController::toggleWater() {
    if (m_water_timer) {
        playClickSound();
        m_water_timer->toggle();
    }
}

void WellnessController::acknowledgeWater() {
    if (m_water_timer) {
        playClickSound();
        m_water_timer->acknowledgeReminder();
        if (onCountersChanged) {
            onCountersChanged();
        }
    }
}

void WellnessController::resetWaterDaily() {
    if (m_water_timer) {
        m_water_timer->resetDailyCounters();
        m_water_timer->reset();
        if (onCountersChanged) {
            onCountersChanged();
        }
    }
}

// Standup timer controls
void WellnessController::toggleStandup() {
    if (m_standup_timer) {
        playClickSound();
        m_standup_timer->toggle();
    }
}

void WellnessController::acknowledgeStandup() {
    if (m_standup_timer) {
        // Skip this reminder
        m_standup_timer->acknowledgeReminder();
        m_standup_timer->reset();
        m_standup_timer->start();
    }
}

void WellnessController::startStandupBreak() {
    if (m_standup_timer) {
        playClickSound();
        m_standup_timer->startBreak();
    }
}

void WellnessController::endStandupBreak() {
    if (m_standup_timer) {
        playClickSound();
        m_standup_timer->endBreak();
        if (onCountersChanged) {
            onCountersChanged();
        }
    }
}

// Eye care timer controls
void WellnessController::toggleEyeCare() {
    if (m_eye_care_timer) {
        playClickSound();
        m_eye_care_timer->toggle();
    }
}

void WellnessController::acknowledgeEyeCare() {
    if (m_eye_care_timer) {
        // Skip this reminder
        m_eye_care_timer->acknowledgeReminder();
        m_eye_care_timer->reset();
        m_eye_care_timer->start();
    }
}

void WellnessController::startEyeCareBreak() {
    if (m_eye_care_timer) {
        playClickSound();
        m_eye_care_timer->startBreak();
    }
}

void WellnessController::endEyeCareBreak() {
    if (m_eye_care_timer) {
        playClickSound();
        m_eye_care_timer->endBreak();
        if (onCountersChanged) {
            onCountersChanged();
        }
    }
}

WellnessCounters WellnessController::getCounters() const {
    WellnessCounters counters;
    if (m_water_timer) {
        counters.water_glasses = m_water_timer->getCompletedCount();
    }
    if (m_standup_timer) {
        counters.standups_completed = m_standup_timer->getCompletedCount();
    }
    if (m_eye_care_timer) {
        counters.eye_breaks_completed = m_eye_care_timer->getCompletedCount();
    }
    return counters;
}

void WellnessController::applySettings(int water_interval_mins, int water_goal, int standup_interval_mins,
                                       int standup_duration_mins, int eye_interval_mins, int eye_break_secs) {
    if (m_water_timer) {
        m_water_timer->setIntervalSeconds(water_interval_mins * SECONDS_PER_MINUTE);
    }
    m_water_daily_goal = water_goal;

    if (m_standup_timer) {
        m_standup_timer->setIntervalSeconds(standup_interval_mins * SECONDS_PER_MINUTE);
        m_standup_timer->setBreakDurationSeconds(standup_duration_mins * SECONDS_PER_MINUTE);
    }

    if (m_eye_care_timer) {
        m_eye_care_timer->setIntervalSeconds(eye_interval_mins * SECONDS_PER_MINUTE);
        m_eye_care_timer->setBreakDurationSeconds(eye_break_secs);
    }
}

void WellnessController::playClickSound() {
    if (m_audio && m_audio->isInitialized()) {
        m_audio->playClickSound();
    }
}

void WellnessController::playBellSound() {
    if (m_audio && m_audio->isInitialized()) {
        m_audio->playBellSound();
    }
}

} // namespace WorkBalance::Controllers
