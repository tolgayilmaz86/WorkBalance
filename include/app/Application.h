#pragma once

#include <memory>

namespace WorkBalance::App {
class Application {
  public:
    /// @param launched_at_startup Set to true when app is auto-started by Windows (via --startup flag)
    explicit Application(bool launched_at_startup = false);
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
