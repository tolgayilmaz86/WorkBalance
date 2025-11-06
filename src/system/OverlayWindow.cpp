#include <system/OverlayWindow.h>

#include <GLFW/glfw3.h>
#include <stdexcept>

namespace WorkBalance::System {
OverlayWindow::OverlayWindow() {
    configureWindowHints();

    m_window = glfwCreateWindow(200, 80, "Timer Overlay", nullptr, nullptr);
    if (m_window == nullptr) {
        throw std::runtime_error("Failed to create overlay window");
    }

    glfwSetWindowPos(m_window, 100, 100);
    glfwHideWindow(m_window);
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

void OverlayWindow::show() {
    if ((m_window != nullptr) && !m_visible) {
        glfwShowWindow(m_window);
        m_visible = true;
    }
}

void OverlayWindow::hide() {
    if ((m_window != nullptr) && m_visible) {
        glfwHideWindow(m_window);
        m_visible = false;
    }
}

} // namespace WorkBalance::System
