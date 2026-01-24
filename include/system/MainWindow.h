#pragma once

#include "WindowBase.h"

#include <string_view>

namespace WorkBalance::System {

class MainWindow final : public WindowBase {
  public:
    MainWindow(int width, int height, std::string_view title);

    void setOverlayMode(bool overlay_mode);

    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

    MainWindow(MainWindow&& other) noexcept;
    MainWindow& operator=(MainWindow&& other) noexcept;

  private:
    void setupOpenGLContext() const;
    void centerOnMonitor(int width, int height) const;
    void applyRoundedCorners() const;
    void setWindowIcon() const;
    void resizeForOverlay() const;
    void resizeForNormal() const;
    [[nodiscard]] int getFullHeight() const;

    // Saved position for restoring after overlay mode
    int m_saved_normal_x = -1;
    int m_saved_normal_y = -1;
    int m_saved_overlay_x = -1;
    int m_saved_overlay_y = -1;
};

} // namespace WorkBalance::System
