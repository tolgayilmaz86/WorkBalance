#include <system/WindowBase.h>

#include <iostream>
#include <stdexcept>

namespace WorkBalance::System {

GLFWManager::GLFWManager() {
    glfwSetErrorCallback(
        [](int error, const char* description) { std::cerr << "GLFW Error " << error << ": " << description << '\n'; });

    if (glfwInit() == 0) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
}

GLFWManager::~GLFWManager() {
    glfwTerminate();
}

WindowBase::~WindowBase() {
    if (m_window != nullptr) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

GLFWwindow* WindowBase::get() const noexcept {
    return m_window;
}

bool WindowBase::shouldClose() const noexcept {
    return m_window != nullptr && glfwWindowShouldClose(m_window) != 0;
}

void WindowBase::swapBuffers() const noexcept {
    if (m_window != nullptr) {
        glfwSwapBuffers(m_window);
    }
}

std::pair<int, int> WindowBase::getFramebufferSize() const noexcept {
    if (m_window != nullptr) {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        return {width, height};
    }
    return {0, 0};
}

std::pair<int, int> WindowBase::getPosition() const noexcept {
    if (m_window != nullptr) {
        int x = 0;
        int y = 0;
        glfwGetWindowPos(m_window, &x, &y);
        return {x, y};
    }
    return {0, 0};
}

void WindowBase::setPosition(int x, int y) noexcept {
    if (m_window != nullptr) {
        glfwSetWindowPos(m_window, x, y);
    }
}

} // namespace WorkBalance::System
