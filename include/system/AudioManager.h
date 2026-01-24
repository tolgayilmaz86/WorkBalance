#pragma once

#include "IAudioService.h"
#include "assets/sounds/miniaudio.h"

#include <cstddef>
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

  private:
    void playEmbeddedSound(const unsigned char* data, size_t size);

    ma_engine m_engine{};
    bool m_initialized = false;
};

std::unique_ptr<IAudioService> createAudioService();

} // namespace WorkBalance::System
