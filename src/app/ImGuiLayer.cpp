#include <app/ImGuiLayer.h>

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <string_view>

#include "assets/embedded_resources.h"
#include "assets/fonts/IconsFontAwesome5Pro.h"
#include <core/Configuration.h>

namespace WorkBalance::App {
ImGuiLayer::ImGuiLayer(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    loadFonts(io);
    applyStyle();

    if (window != nullptr) {
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(Core::Configuration::GLSL_VERSION.data());
        m_owns_backends = true;
    }

    m_initialized = true;
}

ImGuiLayer::~ImGuiLayer() {
    if (!m_initialized) {
        return;
    }

    if (m_owns_backends) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }

    ImGui::DestroyContext();
}

void ImGuiLayer::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::loadFonts(ImGuiIO& io) {
    ImGui::StyleColorsDark();

    ImFontConfig base_config{};
    base_config.FontDataOwnedByAtlas = false;

    const auto addFont = [&](ImFontConfig config, const unsigned char* data, size_t data_size, float font_size,
                             std::string_view warning, const ImWchar* ranges = nullptr) -> ImFont* {
        ImFont* font =
            io.Fonts->AddFontFromMemoryTTF(static_cast<void*>(const_cast<unsigned char*>(data)),
                                           static_cast<int>(data_size), font_size, &config, ranges);

        if (font == nullptr && !warning.empty()) {
            std::cerr << "Warning: " << warning << '\n';
        }
        return font;
    };

    ImFontConfig roboto_config = base_config;
    m_large_font = addFont(roboto_config, roboto_medium_data, roboto_medium_data_size,
                           Core::Configuration::REGULAR_FONT_SIZE,
                           "Failed to load embedded Roboto font. Using default font.");
    if (m_large_font == nullptr) {
        m_large_font = io.Fonts->AddFontDefault();
    }

    constexpr ImWchar icons_ranges[] = {0xf000, 0xf8ff, 0};
    ImFontConfig icons_config = base_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = Core::Configuration::REGULAR_FONT_SIZE;
    addFont(icons_config, fontawesome_data, fontawesome_data_size, Core::Configuration::REGULAR_FONT_SIZE,
            "Failed to load embedded FontAwesome", icons_ranges);

    ImFontConfig formula_config = base_config;
    m_timer_font = addFont(formula_config, formula1_bold_data, formula1_bold_data_size,
                           Core::Configuration::TIMER_FONT_SIZE, "Failed to load embedded Formula1-Bold font");
    m_button_font = addFont(formula_config, formula1_wide_data, formula1_wide_data_size,
                            Core::Configuration::BUTTON_FONT_SIZE, "Failed to load embedded Formula1-Wide font");
    m_overlay_font = addFont(formula_config, formula1_regular_data, formula1_regular_data_size,
                             Core::Configuration::OVERLAY_FONT_SIZE, "Failed to load embedded Formula1-Regular font");

    if (!io.Fonts->Build()) {
        std::cerr << "Error: Failed to build ImGui font atlas\n";
    }
}

void ImGuiLayer::applyStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = Core::Configuration::WINDOW_ROUNDING;
    style.FrameRounding = Core::Configuration::FRAME_ROUNDING;
    style.PopupRounding = Core::Configuration::FRAME_ROUNDING;
    style.ScrollbarRounding = Core::Configuration::FRAME_ROUNDING;
    style.GrabRounding = Core::Configuration::FRAME_ROUNDING;
    style.TabRounding = Core::Configuration::FRAME_ROUNDING;
    style.WindowPadding = ImVec2(20.0f, 20.0f);
    style.FramePadding = ImVec2(10.0f, 8.0f);
    style.ItemSpacing = ImVec2(10.0f, 10.0f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
    colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.3f);
    colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}

} // namespace WorkBalance::App
