#pragma once

#include "WindowBase.h"
#include <core/Configuration.h>

namespace WorkBalance::System {
class OverlayWindow final : public WindowBase {
  public:
    OverlayWindow();
    ~OverlayWindow() override = default;

    OverlayWindow(const OverlayWindow&) = delete;
    OverlayWindow& operator=(const OverlayWindow&) = delete;
    OverlayWindow(OverlayWindow&&) = delete;
    OverlayWindow& operator=(OverlayWindow&&) = delete;

    [[nodiscard]] bool isVisible() const noexcept {
        return m_visible;
    }

    void show();
    void hide();

  private:
    static void configureWindowHints();

    bool m_visible = false;
};

} // namespace WorkBalance::System
