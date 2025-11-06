#pragma once

#include <app/ImGuiLayer.h>
#include <core/Timer.h>
#include <system/OverlayWindow.h>
#include <ui/AppState.h>

namespace WorkBalance::App::UI {
class OverlayView {
  public:
    OverlayView(App::ImGuiLayer& imgui, Core::Timer& timer, AppState& state)
        : m_imgui(imgui), m_timer(timer), m_state(state) {
    }

    void renderContent(System::OverlayWindow& overlay_window);
    void renderFrame(System::OverlayWindow& overlay_window);

  private:
    App::ImGuiLayer& m_imgui;
    Core::Timer& m_timer;
    AppState& m_state;
};

} // namespace WorkBalance::App::UI
