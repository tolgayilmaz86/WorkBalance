#include "app/Application.h"
#include <exception>
#include <iostream>
#include <string_view>

namespace {
bool hasStartupFlag(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string_view arg{argv[i]};
        if (arg == "--startup" || arg == "-startup" || arg == "/startup") {
            return true;
        }
    }
    return false;
}
} // namespace

int main(int argc, char* argv[]) {
    try {
        const bool launched_at_startup = hasStartupFlag(argc, argv);
        WorkBalance::App::Application app{launched_at_startup};
        app.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
