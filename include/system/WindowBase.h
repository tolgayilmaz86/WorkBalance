#pragma once

#include <GLFW/glfw3.h>
#include <utility>

namespace WorkBalance::System {
// RAII wrapper for GLFW - follows Resource Acquisition Is Initialization
class GLFWManager {
  public:
    GLFWManager();
    ~GLFWManager();

    // Non-copyable, non-movable (Singleton-like semantics)
    GLFWManager(const GLFWManager&) = delete;
    GLFWManager& operator=(const GLFWManager&) = delete;
    GLFWManager(GLFWManager&&) = delete;
    GLFWManager& operator=(GLFWManager&&) = delete;
};

// Base window class - common functionality
class WindowBase {
  public:
    virtual ~WindowBase();

    [[nodiscard]] GLFWwindow* get() const noexcept;
    [[nodiscard]] bool shouldClose() const noexcept;

    void swapBuffers() const noexcept;

    [[nodiscard]] std::pair<int, int> getFramebufferSize() const noexcept;

    [[nodiscard]] std::pair<int, int> getPosition() const noexcept;

    void setPosition(int x, int y) noexcept;

  protected:
    GLFWwindow* m_window = nullptr;
};

} // namespace WorkBalance::System
