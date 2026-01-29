#pragma once

#include "IAudioService.h"
#include "assets/sounds/miniaudio.h"

#include <cstddef>
#include <filesystem>
#include <memory>

namespace WorkBalance::System {
class AudioManager final : public IAudioService {
  public:
    AudioManager();
    ~AudioManager() override;

    // Non-copyable, non-movable
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    AudioManager(AudioManager&&) = delete;
    AudioManager& operator=(AudioManager&&) = delete;

    void playClickSound() override;
    void playBellSound() override;
    void playHydrationSound() override;
    void playWalkSound() override;
    [[nodiscard]] bool isInitialized() const noexcept override;
    void setVolume(int volume) override;
    [[nodiscard]] int getVolume() const noexcept override;

  private:
    void playEmbeddedSound(const unsigned char* data, size_t size);
    void stopNotificationSounds();
    void cleanupTempFile();

    ma_engine m_engine{};
    ma_sound m_notification_sound{};
    std::filesystem::path m_current_temp_path;
    bool m_notification_sound_initialized = false;
    bool m_initialized = false;
    int m_volume = 100; // 0-100 percentage
};

std::unique_ptr<IAudioService> createAudioService();

} // namespace WorkBalance::System
