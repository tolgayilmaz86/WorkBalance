#include "system/NotificationManager.h"

#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#include <wintoastlib.h>
using namespace WinToastLib;

namespace {

// Convert UTF-8 string to UTF-16 wstring
std::wstring utf8ToWide(std::string_view utf8) {
    if (utf8.empty()) {
        return {};
    }
    const int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    if (size_needed <= 0) {
        return {};
    }
    std::wstring result(static_cast<size_t>(size_needed), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), result.data(), size_needed);
    return result;
}

class ToastHandler : public IWinToastHandler {
  public:
    void toastActivated() const override {
        // User clicked the notification - could bring window to focus
    }

    void toastActivated(int /*actionIndex*/) const override {
        // User clicked an action button
    }

    void toastActivated(std::wstring /*args*/) const override {
        // User activated toast with arguments
    }

    void toastDismissed(WinToastDismissalReason /*state*/) const override {
        // Notification was dismissed
    }

    void toastFailed() const override {
        std::cerr << "Toast notification failed to show\n";
    }
};

} // namespace

#endif // _WIN32

namespace WorkBalance::System {

NotificationManager::NotificationManager() = default;

NotificationManager::~NotificationManager() = default;

bool NotificationManager::initialize() {
#ifdef _WIN32
    if (m_initialized) {
        return m_supported;
    }

    m_initialized = true;

    if (!WinToast::isCompatible()) {
        std::cerr << "Windows Toast notifications are not supported on this system\n";
        m_supported = false;
        return false;
    }

    WinToast::instance()->setAppName(L"WorkBalance");

    const auto aumi = WinToast::configureAUMI(L"WorkBalance", L"WorkBalance", L"Timer", L"1.0");
    WinToast::instance()->setAppUserModelId(aumi);

    if (!WinToast::instance()->initialize()) {
        std::cerr << "Failed to initialize Windows Toast notifications\n";
        m_supported = false;
        return false;
    }

    m_supported = true;
    return true;
#else
    m_initialized = true;
    m_supported = false;
    return false;
#endif
}

bool NotificationManager::isSupported() const noexcept {
    return m_supported;
}

void NotificationManager::showNotification(std::string_view title, std::string_view message) {
#ifdef _WIN32
    if (!m_supported) {
        return;
    }

    WinToastTemplate templ(WinToastTemplate::Text02);
    templ.setTextField(utf8ToWide(title), WinToastTemplate::FirstLine);
    templ.setTextField(utf8ToWide(message), WinToastTemplate::SecondLine);
    templ.setDuration(WinToastTemplate::Short);

    (void)WinToast::instance()->showToast(templ, new ToastHandler());
#else
    (void)title;
    (void)message;
#endif
}

void NotificationManager::showPomodoroComplete() {
    showNotification("Pomodoro Complete! \xF0\x9F\x8E\x89", "Great work! Time for a well-deserved break.");
}

void NotificationManager::showShortBreakComplete() {
    showNotification("Break's Over! \xF0\x9F\x92\xAA", "Ready to focus? Let's get back to work!");
}

void NotificationManager::showLongBreakComplete() {
    showNotification("Long Break Complete! \xE2\x9C\xA8", "Feeling refreshed? Time to start a new cycle!");
}

void NotificationManager::showWaterReminder() {
    showNotification("Stay Hydrated! \xF0\x9F\x92\xA7", "Time to drink some water. Your body will thank you!");
}

void NotificationManager::showStandupReminder() {
    showNotification("Time to Move! \xF0\x9F\x9A\xB6", "Stand up, stretch, and take a short walk.");
}

void NotificationManager::showEyeCareReminder() {
    showNotification("Eye Break! \xF0\x9F\x91\x80", "Look at something 20 feet away for 20 seconds.");
}

std::unique_ptr<INotificationService> createNotificationService() {
    auto service = std::make_unique<NotificationManager>();
    (void)service->initialize();
    return service;
}

} // namespace WorkBalance::System
