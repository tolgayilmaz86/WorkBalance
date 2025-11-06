#pragma once

#include <memory>

namespace WorkBalance::App {
class Application {
  public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) noexcept;
    Application& operator=(Application&&) noexcept;

    void run();

  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace WorkBalance::App
