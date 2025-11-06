#include <system/MainWindow.h>

#include <core/Configuration.h>
#include "assets/embedded_resources.h"
#include "assets/icons/stb_image.h"

#include <stdexcept>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

namespace WorkBalance::System {

MainWindow::MainWindow(int width, int height, std::string_view title) {
    setupOpenGLContext();

    m_window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    if (m_window == nullptr) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    centerOnMonitor(width, height);
    applyRoundedCorners();
    setWindowIcon();

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0);
}

void MainWindow::setOverlayMode(bool overlay_mode) {
    if (m_window == nullptr) {
        return;
    }

    glfwSetWindowAttrib(m_window, GLFW_FLOATING, overlay_mode ? GLFW_TRUE : GLFW_FALSE);

    if (overlay_mode) {
        resizeForOverlay();
    } else {
        resizeForNormal();
    }
}

MainWindow::MainWindow(MainWindow&& other) noexcept : WindowBase() {
    m_window = other.m_window;
    other.m_window = nullptr;
}

MainWindow& MainWindow::operator=(MainWindow&& other) noexcept {
    if (this != &other) {
        if (m_window != nullptr) {
            glfwDestroyWindow(m_window);
        }
        m_window = other.m_window;
        other.m_window = nullptr;
    }
    return *this;
}

void MainWindow::setupOpenGLContext() const {
    using namespace Core;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Configuration::GL_MAJOR_VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Configuration::GL_MINOR_VERSION);

    if constexpr (Configuration::USE_CORE_PROFILE) {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
}

void MainWindow::centerOnMonitor(int width, int height) const {
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    if (primary_monitor == nullptr) {
        return;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
    if (mode == nullptr) {
        return;
    }

    int monitor_x = 0;
    int monitor_y = 0;
    glfwGetMonitorPos(primary_monitor, &monitor_x, &monitor_y);

    const int window_x = monitor_x + ((mode->width - width) / 2);
    const int window_y = monitor_y + ((mode->height - height) / 2);

    glfwSetWindowPos(m_window, window_x, window_y);
}

void MainWindow::applyRoundedCorners() const {
#ifdef _WIN32
    if (!m_window) {
        return;
    }

    HWND hwnd = glfwGetWin32Window(m_window);
    if (hwnd == nullptr) {
        return;
    }

    enum DWM_WINDOW_CORNER_PREFERENCE {
        DWMWCP_DEFAULT = 0,
        DWMWCP_DONOTROUND = 1,
        DWMWCP_ROUND = 2,
        DWMWCP_ROUNDSMALL = 3
    };

    const DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_ROUND;
    DwmSetWindowAttribute(hwnd, 33, &preference, sizeof(preference));
#endif
}

void MainWindow::setWindowIcon() const {
    if (!m_window) {
        return;
    }

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* pixels = stbi_load_from_memory(app_icon_png_data, static_cast<int>(app_icon_png_data_size), &width,
                                                  &height, &channels, 4);

    if (pixels == nullptr) {
        return;
    }

    GLFWimage icon{};
    icon.width = width;
    icon.height = height;
    icon.pixels = pixels;

    glfwSetWindowIcon(m_window, 1, &icon);
    stbi_image_free(pixels);
}

void MainWindow::resizeForOverlay() const {
    constexpr int overlay_width = 150;
    constexpr int overlay_height = 70;
    glfwSetWindowSize(m_window, overlay_width, overlay_height);

    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    if (primary_monitor == nullptr) {
        return;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
    if (mode == nullptr) {
        return;
    }

    int monitor_x = 0;
    int monitor_y = 0;
    glfwGetMonitorPos(primary_monitor, &monitor_x, &monitor_y);

    const int window_x = monitor_x + ((mode->width - overlay_width) / 2);
    const int window_y = monitor_y + 10;

    glfwSetWindowPos(m_window, window_x, window_y);
}

void MainWindow::resizeForNormal() const {
    using namespace Core;
    const int height = getFullHeight();
    glfwSetWindowSize(m_window, Configuration::DEFAULT_WINDOW_WIDTH, height);

    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    if (primary_monitor == nullptr) {
        return;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
    if (mode == nullptr) {
        return;
    }

    int monitor_x = 0;
    int monitor_y = 0;
    glfwGetMonitorPos(primary_monitor, &monitor_x, &monitor_y);

    const int window_x = monitor_x + ((mode->width - Configuration::DEFAULT_WINDOW_WIDTH) / 2);
    const int window_y = monitor_y + ((mode->height - height) / 2);

    glfwSetWindowPos(m_window, window_x, window_y);
}

int MainWindow::getFullHeight() const {
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    if (primary_monitor != nullptr) {
        const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
        if (mode != nullptr) {
            constexpr int taskbar_height = 90;
            return mode->height - taskbar_height;
        }
    }
    return 600;
}

} // namespace WorkBalance::System
