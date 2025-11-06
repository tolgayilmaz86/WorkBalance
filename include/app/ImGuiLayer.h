#pragma once

#include <imgui.h>

struct GLFWwindow;

namespace WorkBalance::App {
class ImGuiLayer {
  public:
    explicit ImGuiLayer(GLFWwindow* window);
    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;
    ImGuiLayer(ImGuiLayer&&) = delete;
    ImGuiLayer& operator=(ImGuiLayer&&) = delete;

    static void newFrame();
    static void render();

    [[nodiscard]] ImFont* largeFont() const noexcept {
        return m_large_font;
    }
    [[nodiscard]] ImFont* timerFont() const noexcept {
        return m_timer_font;
    }
    [[nodiscard]] ImFont* buttonFont() const noexcept {
        return m_button_font;
    }
    [[nodiscard]] ImFont* overlayFont() const noexcept {
        return m_overlay_font;
    }

  private:
    void loadFonts(ImGuiIO& io);
    static void applyStyle();

    bool m_initialized = false;
    bool m_owns_backends = false;
    ImFont* m_large_font = nullptr;
    ImFont* m_timer_font = nullptr;
    ImFont* m_button_font = nullptr;
    ImFont* m_overlay_font = nullptr;
};

} // namespace WorkBalance::App
