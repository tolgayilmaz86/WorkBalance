#include <system/SystemTray.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>

#include "assets/embedded_resources.h"
#include "assets/icons/stb_image.h"
#endif

#include <array>
#include <cstring>

namespace WorkBalance::System {

#ifdef _WIN32
namespace {
constexpr UINT WM_TRAYICON = WM_USER + 1;
constexpr UINT ID_TRAY_ICON = 1;

// Menu item IDs
constexpr UINT ID_MENU_TOGGLE_TIMER = 1001;
constexpr UINT ID_MENU_TOGGLE_MODE = 1002;
constexpr UINT ID_MENU_SHOW_WINDOW = 1003;
constexpr UINT ID_MENU_QUIT = 1004;

constexpr const wchar_t* WINDOW_CLASS_NAME = L"WorkBalanceTrayClass";
constexpr const wchar_t* DEFAULT_TOOLTIP = L"WorkBalance";
} // namespace

class SystemTray::Impl {
  public:
    Impl() = default;
    ~Impl() {
        cleanup();
    }

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    bool initialize(SystemTrayCallbacks callbacks) {
        m_callbacks = std::move(callbacks);

        if (!registerWindowClass()) {
            return false;
        }

        if (!createMessageWindow()) {
            return false;
        }

        if (!createTrayIcon()) {
            return false;
        }

        m_initialized = true;
        return true;
    }

    void setTooltip(std::string_view text) {
        if (!m_initialized) {
            return;
        }

        // Convert UTF-8 to wide string
        std::array<wchar_t, 128> wide_text{};
        MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), wide_text.data(),
                            static_cast<int>(wide_text.size()) - 1);

        NOTIFYICONDATAW nid{};
        nid.cbSize = sizeof(nid);
        nid.hWnd = m_message_window;
        nid.uID = ID_TRAY_ICON;
        nid.uFlags = NIF_TIP;
        wcscpy_s(nid.szTip, wide_text.data());

        Shell_NotifyIconW(NIM_MODIFY, &nid);
    }

    void updateTimerState(bool is_running) {
        m_timer_running = is_running;
    }

    void updateWindowMode(bool is_overlay) {
        m_is_overlay_mode = is_overlay;
    }

    void processMessages() {
        MSG msg;
        while (PeekMessageW(&msg, m_message_window, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    [[nodiscard]] bool isInitialized() const noexcept {
        return m_initialized;
    }

  private:
    bool registerWindowClass() {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = windowProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = WINDOW_CLASS_NAME;

        if (RegisterClassExW(&wc) == 0) {
            // Class might already be registered
            if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
                return false;
            }
        }
        return true;
    }

    bool createMessageWindow() {
        m_message_window = CreateWindowExW(0, WINDOW_CLASS_NAME, L"WorkBalance Tray", 0, 0, 0, 0, 0, HWND_MESSAGE,
                                           nullptr, GetModuleHandleW(nullptr), this);
        return m_message_window != nullptr;
    }

    bool createTrayIcon() {
        NOTIFYICONDATAW nid{};
        nid.cbSize = sizeof(nid);
        nid.hWnd = m_message_window;
        nid.uID = ID_TRAY_ICON;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON;
        nid.hIcon = loadIconFromEmbeddedResource();
        wcscpy_s(nid.szTip, DEFAULT_TOOLTIP);

        if (!Shell_NotifyIconW(NIM_ADD, &nid)) {
            if (nid.hIcon != nullptr) {
                DestroyIcon(nid.hIcon);
            }
            return false;
        }

        m_icon = nid.hIcon;
        return true;
    }

    static HICON loadIconFromEmbeddedResource() {
        int width = 0;
        int height = 0;
        int channels = 0;

        unsigned char* pixels = stbi_load_from_memory(app_icon_png_data, static_cast<int>(app_icon_png_data_size),
                                                      &width, &height, &channels, 4);
        if (pixels == nullptr) {
            return LoadIconW(nullptr, MAKEINTRESOURCEW(32512)); // IDI_APPLICATION
        }

        // Convert RGBA to BGRA for Windows
        for (int i = 0; i < width * height; ++i) {
            std::swap(pixels[i * 4 + 0], pixels[i * 4 + 2]);
        }

        HICON icon = nullptr;
        ICONINFO icon_info{};
        icon_info.fIcon = TRUE;

        // Create bitmap for the icon
        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height; // Top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        HDC hdc = GetDC(nullptr);
        void* bits = nullptr;
        HBITMAP color_bitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (color_bitmap != nullptr && bits != nullptr) {
            memcpy(bits, pixels, static_cast<size_t>(width * height * 4));

            // Create mask bitmap
            HBITMAP mask_bitmap = CreateBitmap(width, height, 1, 1, nullptr);
            if (mask_bitmap != nullptr) {
                icon_info.hbmColor = color_bitmap;
                icon_info.hbmMask = mask_bitmap;
                icon = CreateIconIndirect(&icon_info);
                DeleteObject(mask_bitmap);
            }
            DeleteObject(color_bitmap);
        }
        ReleaseDC(nullptr, hdc);
        stbi_image_free(pixels);

        return icon != nullptr ? icon : LoadIconW(nullptr, MAKEINTRESOURCEW(32512)); // IDI_APPLICATION
    }

    void showContextMenu() {
        HMENU menu = CreatePopupMenu();
        if (menu == nullptr) {
            return;
        }

        // Timer toggle
        const wchar_t* timer_text = m_timer_running ? L"Pause Timer" : L"Start Timer";
        AppendMenuW(menu, MF_STRING, ID_MENU_TOGGLE_TIMER, timer_text);

        // Mode toggle
        const wchar_t* mode_text = m_is_overlay_mode ? L"Switch to Window Mode" : L"Switch to Overlay Mode";
        AppendMenuW(menu, MF_STRING, ID_MENU_TOGGLE_MODE, mode_text);

        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

        // Show window
        AppendMenuW(menu, MF_STRING, ID_MENU_SHOW_WINDOW, L"Show Window");

        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

        // Quit
        AppendMenuW(menu, MF_STRING, ID_MENU_QUIT, L"Quit");

        // Get cursor position and show menu
        POINT pt;
        GetCursorPos(&pt);

        // Required for proper menu dismissal
        SetForegroundWindow(m_message_window);

        TrackPopupMenu(menu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_message_window,
                       nullptr);

        PostMessageW(m_message_window, WM_NULL, 0, 0);
        DestroyMenu(menu);
    }

    void handleMenuCommand(UINT command_id) {
        switch (command_id) {
            case ID_MENU_TOGGLE_TIMER:
                if (m_callbacks.onToggleTimer) {
                    m_callbacks.onToggleTimer();
                }
                break;
            case ID_MENU_TOGGLE_MODE:
                if (m_callbacks.onToggleOverlayMode) {
                    m_callbacks.onToggleOverlayMode();
                }
                break;
            case ID_MENU_SHOW_WINDOW:
                if (m_callbacks.onShowWindow) {
                    m_callbacks.onShowWindow();
                }
                break;
            case ID_MENU_QUIT:
                if (m_callbacks.onQuit) {
                    m_callbacks.onQuit();
                }
                break;
            default:
                break;
        }
    }

    void cleanup() {
        if (m_message_window != nullptr) {
            NOTIFYICONDATAW nid{};
            nid.cbSize = sizeof(nid);
            nid.hWnd = m_message_window;
            nid.uID = ID_TRAY_ICON;
            Shell_NotifyIconW(NIM_DELETE, &nid);

            DestroyWindow(m_message_window);
            m_message_window = nullptr;
        }

        if (m_icon != nullptr) {
            DestroyIcon(m_icon);
            m_icon = nullptr;
        }

        UnregisterClassW(WINDOW_CLASS_NAME, GetModuleHandleW(nullptr));
        m_initialized = false;
    }

    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        Impl* self = nullptr;

        if (msg == WM_NCCREATE) {
            auto* create_struct = reinterpret_cast<CREATESTRUCTW*>(lParam);
            self = static_cast<Impl*>(create_struct->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        } else {
            self = reinterpret_cast<Impl*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (self == nullptr) {
            return DefWindowProcW(hwnd, msg, wParam, lParam);
        }

        switch (msg) {
            case WM_TRAYICON:
                if (LOWORD(lParam) == WM_RBUTTONUP || LOWORD(lParam) == WM_CONTEXTMENU) {
                    self->showContextMenu();
                } else if (LOWORD(lParam) == WM_LBUTTONDBLCLK) {
                    if (self->m_callbacks.onShowWindow) {
                        self->m_callbacks.onShowWindow();
                    }
                }
                return 0;

            case WM_COMMAND:
                self->handleMenuCommand(LOWORD(wParam));
                return 0;

            default:
                break;
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    SystemTrayCallbacks m_callbacks;
    HWND m_message_window = nullptr;
    HICON m_icon = nullptr;
    bool m_initialized = false;
    bool m_timer_running = false;
    bool m_is_overlay_mode = false;
};

#else
// Stub implementation for non-Windows platforms
class SystemTray::Impl {
  public:
    bool initialize(SystemTrayCallbacks /*callbacks*/) {
        return false;
    }
    void setTooltip(std::string_view /*text*/) {
    }
    void updateTimerState(bool /*is_running*/) {
    }
    void updateWindowMode(bool /*is_overlay*/) {
    }
    void processMessages() {
    }
    [[nodiscard]] bool isInitialized() const noexcept {
        return false;
    }
};
#endif

// SystemTray public interface implementation
SystemTray::SystemTray() : m_impl(std::make_unique<Impl>()) {
}

SystemTray::~SystemTray() = default;

SystemTray::SystemTray(SystemTray&&) noexcept = default;
SystemTray& SystemTray::operator=(SystemTray&&) noexcept = default;

bool SystemTray::initialize(SystemTrayCallbacks callbacks) {
    return m_impl->initialize(std::move(callbacks));
}

void SystemTray::setTooltip(std::string_view text) {
    m_impl->setTooltip(text);
}

void SystemTray::updateTimerState(bool is_running) {
    m_impl->updateTimerState(is_running);
}

void SystemTray::updateWindowMode(bool is_overlay) {
    m_impl->updateWindowMode(is_overlay);
}

void SystemTray::processMessages() {
    m_impl->processMessages();
}

bool SystemTray::isInitialized() const noexcept {
    return m_impl->isInitialized();
}

} // namespace WorkBalance::System
