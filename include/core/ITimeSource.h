#pragma once

#include <chrono>
#include <memory>

namespace WorkBalance::Core {

/// @brief Interface for obtaining current time, enabling testability
class ITimeSource {
  public:
    virtual ~ITimeSource() = default;

    /// @brief Get the current time point
    [[nodiscard]] virtual std::chrono::steady_clock::time_point now() const = 0;
};

/// @brief Production implementation using system steady clock
class SystemTimeSource final : public ITimeSource {
  public:
    [[nodiscard]] std::chrono::steady_clock::time_point now() const override {
        return std::chrono::steady_clock::now();
    }
};

/// @brief Mock implementation for testing
class MockTimeSource final : public ITimeSource {
  public:
    /// @brief Advance the mock time by a duration
    void advance(std::chrono::steady_clock::duration duration) {
        m_current_time += duration;
    }

    /// @brief Set the mock time to a specific point
    void setTime(std::chrono::steady_clock::time_point time) {
        m_current_time = time;
    }

    [[nodiscard]] std::chrono::steady_clock::time_point now() const override {
        return m_current_time;
    }

  private:
    std::chrono::steady_clock::time_point m_current_time{};
};

/// @brief Factory function to create the default time source
[[nodiscard]] inline std::shared_ptr<ITimeSource> createDefaultTimeSource() {
    return std::make_shared<SystemTimeSource>();
}

} // namespace WorkBalance::Core
