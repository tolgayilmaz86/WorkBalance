#include <system/OverlayWindow.h>

#include <GLFW/glfw3.h>
#include <stdexcept>

namespace WorkBalance::System {
constexpr int DEFAULT_WIDTH = 200;
constexpr int DEFAULT_HEIGHT = 80;
constexpr int START_X = 100;
constexpr int START_Y = 100;
constexpr const char* OVERLAY_TITLE = "Timer Overlay";
OverlayWindow::OverlayWindow() {
    configureWindowHints();

    m_window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, OVERLAY_TITLE, nullptr, nullptr);
    if (m_window == nullptr) {
        throw std::runtime_error("Failed to create overlay window");
    }

    glfwSetWindowPos(m_window, START_X, START_Y);
    glfwHideWindow(m_window);
}

void OverlayWindow::show() {
    if (m_window == nullptr || m_visible) {
        return;
    }

    glfwShowWindow(m_window);
    m_visible = true;
}

void OverlayWindow::hide() {
    if (m_window == nullptr || !m_visible) {
        return;
    }

    glfwHideWindow(m_window);
    m_visible = false;
}

void OverlayWindow::configureWindowHints() {
    using namespace Core;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Configuration::GL_MAJOR_VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Configuration::GL_MINOR_VERSION);
    if constexpr (Configuration::USE_CORE_PROFILE) {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);
}

} // namespace WorkBalance::System
