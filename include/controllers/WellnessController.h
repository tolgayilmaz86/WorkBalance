#pragma once

#include <core/WellnessTimer.h>
#include <core/WellnessTypes.h>
#include <system/IAudioService.h>

#include <functional>
#include <memory>

namespace WorkBalance::Controllers {

/// @brief Counters for wellness activities
struct WellnessCounters {
    int water_glasses = 0;
    int standups_completed = 0;
    int eye_breaks_completed = 0;
};

/// @brief Controller for managing all wellness timers (water, standup, eye care)
class WellnessController {
  public:
    /// @brief Constructs a WellnessController with the three wellness timers
    WellnessController(std::unique_ptr<Core::WellnessTimer> water, std::unique_ptr<Core::WellnessTimer> standup,
                       std::unique_ptr<Core::WellnessTimer> eye_care, System::IAudioService* audio = nullptr);

    /// @brief Update all wellness timers, should be called each frame
    void update();

    // Water timer controls
    void toggleWater();
    void acknowledgeWater();
    void resetWaterDaily();

    // Standup timer controls
    void toggleStandup();
    void acknowledgeStandup();
    void startStandupBreak();
    void endStandupBreak();

    // Eye care timer controls
    void toggleEyeCare();
    void acknowledgeEyeCare();
    void startEyeCareBreak();
    void endEyeCareBreak();

    /// @brief Get current wellness counters
    [[nodiscard]] WellnessCounters getCounters() const;

    /// @brief Apply wellness settings
    void applySettings(int water_interval_mins, int water_goal, int standup_interval_mins, int standup_duration_mins,
                       int eye_interval_mins, int eye_break_secs);

    // Getters for timers (for UI display)
    [[nodiscard]] Core::WellnessTimer* getWaterTimer() const noexcept {
        return m_water_timer.get();
    }
    [[nodiscard]] Core::WellnessTimer* getStandupTimer() const noexcept {
        return m_standup_timer.get();
    }
    [[nodiscard]] Core::WellnessTimer* getEyeCareTimer() const noexcept {
        return m_eye_care_timer.get();
    }

    // Event callbacks
    std::function<void(Core::WellnessType)> onTimerComplete;
    std::function<void()> onCountersChanged;

  private:
    void handleTimerComplete(Core::WellnessType type);
    void playClickSound();
    void playBellSound();

    std::unique_ptr<Core::WellnessTimer> m_water_timer;
    std::unique_ptr<Core::WellnessTimer> m_standup_timer;
    std::unique_ptr<Core::WellnessTimer> m_eye_care_timer;
    System::IAudioService* m_audio;
    int m_water_daily_goal{8};
};

} // namespace WorkBalance::Controllers
