#pragma once

#include <memory>
#include <string_view>

namespace WorkBalance::System {
// Interface following Dependency Inversion Principle
class IAudioService {
  public:
    virtual ~IAudioService() = default;

    virtual void playClickSound() = 0;
    virtual void playBellSound() = 0;
    virtual void playHydrationSound() = 0;
    virtual void playWalkSound() = 0;
    virtual bool isInitialized() const noexcept = 0;

    /// @brief Set the master volume for audio playback
    /// @param volume Volume level from 0 (mute) to 100 (full volume)
    virtual void setVolume(int volume) = 0;

    /// @brief Get the current master volume
    /// @return Volume level from 0 to 100
    [[nodiscard]] virtual int getVolume() const noexcept = 0;
};

// Factory function to create audio service implementation
[[nodiscard]] std::unique_ptr<IAudioService> createAudioService();

} // namespace WorkBalance::System
