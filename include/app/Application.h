#pragma once

#include <memory>

namespace WorkBalance::App {
class Application {
  public:
    Application();
    ~Application();

    Application(Application&&) noexcept = default;
    Application& operator=(Application&&) noexcept = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /// @brief Runs the main application loop.
    void run();

  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace WorkBalance::App
