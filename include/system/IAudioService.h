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
    virtual bool isInitialized() const noexcept = 0;
};

// Factory function to create audio service implementation
[[nodiscard]] std::unique_ptr<IAudioService> createAudioService();

} // namespace WorkBalance::System
