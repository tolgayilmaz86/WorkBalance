#include "system/WindowsStartup.h"

#ifdef _WIN32
#include <Windows.h>
#include <string>
#endif

namespace WorkBalance::System {

#ifdef _WIN32

std::string WindowsStartup::getStartupCommand() {
    char path[MAX_PATH];
    if (GetModuleFileNameA(nullptr, path, MAX_PATH) == 0) {
        return "";
    }
    // Add --startup flag so the app knows it was launched at Windows startup
    return std::string("\"") + path + "\" --startup";
}

bool WindowsStartup::isRegistered() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, STARTUP_KEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    char value[MAX_PATH];
    DWORD valueSize = sizeof(value);
    DWORD type;
    bool registered =
        (RegQueryValueExA(hKey, APP_NAME, nullptr, &type, reinterpret_cast<BYTE*>(value), &valueSize) == ERROR_SUCCESS);

    RegCloseKey(hKey);
    return registered;
}

bool WindowsStartup::registerStartup() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, STARTUP_KEY, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    std::string command = getStartupCommand();
    if (command.empty()) {
        RegCloseKey(hKey);
        return false;
    }

    bool success = (RegSetValueExA(hKey, APP_NAME, 0, REG_SZ, reinterpret_cast<const BYTE*>(command.c_str()),
                                   static_cast<DWORD>(command.size() + 1)) == ERROR_SUCCESS);

    RegCloseKey(hKey);
    return success;
}

bool WindowsStartup::unregisterStartup() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, STARTUP_KEY, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    // Delete the value - it's OK if it doesn't exist
    RegDeleteValueA(hKey, APP_NAME);
    RegCloseKey(hKey);
    return true;
}

bool WindowsStartup::setStartupEnabled(bool enabled) {
    if (enabled) {
        return registerStartup();
    } else {
        return unregisterStartup();
    }
}

#else
// Non-Windows platforms - stub implementations

std::string WindowsStartup::getStartupCommand() {
    return "";
}

bool WindowsStartup::isRegistered() {
    return false;
}

bool WindowsStartup::registerStartup() {
    return false;
}

bool WindowsStartup::unregisterStartup() {
    return true;
}

bool WindowsStartup::setStartupEnabled(bool /*enabled*/) {
    return false;
}

#endif

} // namespace WorkBalance::System
