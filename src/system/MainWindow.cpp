#include <system/MainWindow.h>

#include <core/Configuration.h>
#include "assets/embedded_resources.h"
#include "assets/icons/stb_image.h"

#include <optional>
#include <stdexcept>
#include <string_view>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

namespace WorkBalance::System {
namespace {
struct MonitorData {
    GLFWmonitor* handle = nullptr;
    const GLFWvidmode* mode = nullptr;
    int x = 0;
    int y = 0;

    [[nodiscard]] bool valid() const noexcept {
        return handle != nullptr && mode != nullptr;
    }
};

[[nodiscard]] std::optional<MonitorData> queryPrimaryMonitor() {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (monitor == nullptr) {
        return std::nullopt;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (mode == nullptr) {
        return std::nullopt;
    }

    MonitorData data{monitor, mode};
    glfwGetMonitorPos(monitor, &data.x, &data.y);
    return data;
}

void positionWindow(GLFWwindow* window, int x, int y) {
    if (window != nullptr) {
        glfwSetWindowPos(window, x, y);
    }
}

void sizeWindow(GLFWwindow* window, int width, int height) {
    if (window != nullptr) {
        glfwSetWindowSize(window, width, height);
    }
}

[[nodiscard]] int calculateWindowHeight(const MonitorData& monitor) {
    constexpr int default_height = 600;
    constexpr int taskbar_height = 90;
    if (!monitor.valid()) {
        return default_height;
    }
    return monitor.mode->height - taskbar_height;
}

[[nodiscard]] MonitorData monitorOrDefault(std::optional<MonitorData> monitor) {
    if (monitor.has_value()) {
        return *monitor;
    }
    return MonitorData{};
}

#ifdef _WIN32
void applyRoundedCornersToWindow(GLFWwindow* window) {
    if (window == nullptr) {
        return;
    }

    HWND hwnd = glfwGetWin32Window(window);
    if (hwnd == nullptr) {
        return;
    }

    enum DWM_WINDOW_CORNER_PREFERENCE {
        DWMWCP_DEFAULT = 0,
        DWMWCP_DONOTROUND = 1,
        DWMWCP_ROUND = 2,
        DWMWCP_ROUNDSMALL = 3
    };

    constexpr DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_ROUND;
    constexpr DWORD attribute = 33;
    DwmSetWindowAttribute(hwnd, attribute, &preference, sizeof(preference));
}
#else
void applyRoundedCornersToWindow(GLFWwindow*) {
}
#endif

void setIcon(GLFWwindow* window) {
    if (window == nullptr) {
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

    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(pixels);
}

void applyOverlaySize(GLFWwindow* window, const MonitorData& monitor) {
    constexpr int overlay_width = 200;
    constexpr int overlay_height = 70;
    sizeWindow(window, overlay_width, overlay_height);

    if (!monitor.valid()) {
        return;
    }

    const int window_x = monitor.x + ((monitor.mode->width - overlay_width) / 2);
    const int window_y = monitor.y + 10;
    positionWindow(window, window_x, window_y);
}

void applyNormalSize(GLFWwindow* window, const MonitorData& monitor) {
    const int height = calculateWindowHeight(monitor);
    sizeWindow(window, Core::Configuration::DEFAULT_WINDOW_WIDTH, height);

    if (!monitor.valid()) {
        return;
    }

    const int window_x = monitor.x + ((monitor.mode->width - Core::Configuration::DEFAULT_WINDOW_WIDTH) / 2);
    const int window_y = monitor.y + ((monitor.mode->height - height) / 2);
    positionWindow(window, window_x, window_y);
}

void centerWindow(GLFWwindow* window, int width, int height) {
    const auto monitor = queryPrimaryMonitor();
    if (!monitor.has_value()) {
        return;
    }

    const int window_x = monitor->x + ((monitor->mode->width - width) / 2);
    const int window_y = monitor->y + ((monitor->mode->height - height) / 2);
    positionWindow(window, window_x, window_y);
}
} // namespace

MainWindow::MainWindow(int width, int height, std::string_view title) {
    setupOpenGLContext();

    m_window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    if (m_window == nullptr) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    centerWindow(m_window, width, height);
    applyRoundedCornersToWindow(m_window);
    setIcon(m_window);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0);
}

void MainWindow::setOverlayMode(bool overlay_mode) {
    if (m_window == nullptr) {
        return;
    }

    glfwSetWindowAttrib(m_window, GLFW_FLOATING, overlay_mode ? GLFW_TRUE : GLFW_FALSE);
    const MonitorData monitor = monitorOrDefault(queryPrimaryMonitor());

    if (overlay_mode) {
        applyOverlaySize(m_window, monitor);
    } else {
        applyNormalSize(m_window, monitor);
    }
}

MainWindow::MainWindow(MainWindow&& other) noexcept : WindowBase() {
    m_window = other.m_window;
    other.m_window = nullptr;
}

MainWindow& MainWindow::operator=(MainWindow&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    if (m_window != nullptr) {
        glfwDestroyWindow(m_window);
    }

    m_window = other.m_window;
    other.m_window = nullptr;
    return *this;
}

void MainWindow::setupOpenGLContext() const {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Core::Configuration::GL_MAJOR_VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Core::Configuration::GL_MINOR_VERSION);

    if constexpr (Core::Configuration::USE_CORE_PROFILE) {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
}

void MainWindow::centerOnMonitor(int width, int height) const {
    centerWindow(m_window, width, height);
}

void MainWindow::applyRoundedCorners() const {
    applyRoundedCornersToWindow(m_window);
}

void MainWindow::setWindowIcon() const {
    setIcon(m_window);
}

void MainWindow::resizeForOverlay() const {
    const MonitorData monitor = monitorOrDefault(queryPrimaryMonitor());
    applyOverlaySize(m_window, monitor);
}

void MainWindow::resizeForNormal() const {
    const MonitorData monitor = monitorOrDefault(queryPrimaryMonitor());
    applyNormalSize(m_window, monitor);
}

int MainWindow::getFullHeight() const {
    const MonitorData monitor = monitorOrDefault(queryPrimaryMonitor());
    return calculateWindowHeight(monitor);
}

} // namespace WorkBalance::System
