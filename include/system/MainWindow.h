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
};

} // namespace WorkBalance::System
