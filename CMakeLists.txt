cmake_minimum_required(VERSION 3.21)

# Set vcpkg toolchain file - use VCPKG_ROOT environment variable if available
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
else()
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

project(WorkBalance VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add /FS flag for MSVC to fix parallel compilation PDB file access issues
# Add /Zc:__cplusplus for correct __cplusplus macro value
if(MSVC)
    add_compile_options(/FS /Zc:__cplusplus)
endif()

# Find required packages
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(imgui CONFIG REQUIRED)

# Include resource embedding script
include(embed_resources.cmake)

# Embed sound files
set(EMBEDDED_RESOURCES_DIR "${CMAKE_CURRENT_BINARY_DIR}/embedded")
file(MAKE_DIRECTORY ${EMBEDDED_RESOURCES_DIR})

embed_resource(
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/sounds/click.wav"
    "${EMBEDDED_RESOURCES_DIR}/click_wav.cpp"
    "click_wav_data"
)

embed_resource(
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/sounds/bell.wav"
    "${EMBEDDED_RESOURCES_DIR}/bell_wav.cpp"
    "bell_wav_data"
)

# Embed font files
embed_resource(
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Roboto-Medium.ttf"
    "${EMBEDDED_RESOURCES_DIR}/roboto_medium.cpp"
    "roboto_medium_data"
)

embed_resource(
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/fa-solid-900.ttf"
    "${EMBEDDED_RESOURCES_DIR}/fontawesome.cpp"
    "fontawesome_data"
)

embed_resource(
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Formula1-Bold.otf"
    "${EMBEDDED_RESOURCES_DIR}/formula1_bold.cpp"
    "formula1_bold_data"
)

embed_resource(
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Formula1-Wide.otf"
    "${EMBEDDED_RESOURCES_DIR}/formula1_wide.cpp"
    "formula1_wide_data"
)

embed_resource(
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Formula1-Regular.otf"
    "${EMBEDDED_RESOURCES_DIR}/formula1_regular.cpp"
    "formula1_regular_data"
)

# Embed app icon (PNG for GLFW window icon)
embed_resource(
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/app_icon.png"
    "${EMBEDDED_RESOURCES_DIR}/app_icon.cpp"
    "app_icon_png_data"
)

# Create executable with embedded resources
if(WIN32)
    # Add resource file for Windows icon
    add_executable(WorkBalance
    main.cpp
    src/app/Application.cpp
    src/app/ImGuiLayer.cpp
    src/controllers/TaskController.cpp
    src/controllers/TimerController.cpp
    src/controllers/WellnessController.cpp
    src/core/Persistence.cpp
    src/core/Task.cpp
    src/core/Timer.cpp
    src/core/WellnessTimer.cpp
    src/app/ui/MainWindowView.cpp
    src/app/ui/OverlayView.cpp
    src/app/ui/WellnessViews.cpp
    src/app/ui/components/SettingsPopup.cpp
    src/app/ui/components/TaskListPanel.cpp
    src/app/ui/components/TimerPanel.cpp
    src/ui/NavigationTabs.cpp
    src/system/AudioManager.cpp
    src/system/MainWindow.cpp
    src/system/StbImage.cpp
    src/system/MiniaudioImplementation.cpp
    src/system/OverlayWindow.cpp
    src/system/SystemTray.cpp
    src/system/WindowBase.cpp
        app.rc
        ${EMBEDDED_RESOURCES_DIR}/click_wav.cpp
        ${EMBEDDED_RESOURCES_DIR}/bell_wav.cpp
        ${EMBEDDED_RESOURCES_DIR}/roboto_medium.cpp
        ${EMBEDDED_RESOURCES_DIR}/fontawesome.cpp
        ${EMBEDDED_RESOURCES_DIR}/formula1_bold.cpp
        ${EMBEDDED_RESOURCES_DIR}/formula1_wide.cpp
        ${EMBEDDED_RESOURCES_DIR}/formula1_regular.cpp
        ${EMBEDDED_RESOURCES_DIR}/app_icon.cpp
    )
    # Hide console window in release mode using linker flags
    target_link_options(WorkBalance PRIVATE
        $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
        $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>
    )
else()
    add_executable(WorkBalance
    main.cpp
    src/app/Application.cpp
    src/app/ImGuiLayer.cpp
    src/controllers/TaskController.cpp
    src/controllers/TimerController.cpp
    src/controllers/WellnessController.cpp
    src/core/Persistence.cpp
    src/core/Task.cpp
    src/core/Timer.cpp
    src/core/WellnessTimer.cpp
    src/app/ui/MainWindowView.cpp
    src/app/ui/OverlayView.cpp
    src/app/ui/WellnessViews.cpp
    src/app/ui/components/SettingsPopup.cpp
    src/app/ui/components/TaskListPanel.cpp
    src/app/ui/components/TimerPanel.cpp
    src/ui/NavigationTabs.cpp
    src/system/AudioManager.cpp
    src/system/MainWindow.cpp
    src/system/StbImage.cpp
    src/system/MiniaudioImplementation.cpp
    src/system/OverlayWindow.cpp
    src/system/SystemTray.cpp
    src/system/WindowBase.cpp
        ${EMBEDDED_RESOURCES_DIR}/click_wav.cpp
        ${EMBEDDED_RESOURCES_DIR}/bell_wav.cpp
        ${EMBEDDED_RESOURCES_DIR}/roboto_medium.cpp
        ${EMBEDDED_RESOURCES_DIR}/fontawesome.cpp
        ${EMBEDDED_RESOURCES_DIR}/formula1_bold.cpp
        ${EMBEDDED_RESOURCES_DIR}/formula1_wide.cpp
        ${EMBEDDED_RESOURCES_DIR}/formula1_regular.cpp
        ${EMBEDDED_RESOURCES_DIR}/app_icon.cpp
    )
endif()

target_include_directories(WorkBalance PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(WorkBalance PRIVATE
    glfw
    OpenGL::GL
    imgui::imgui
)

# Set target properties
set_target_properties(WorkBalance PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Enable additional compiler warnings and optimizations
if(MSVC)
    target_compile_options(WorkBalance PRIVATE
        /W4                     # High warning level
        /permissive-            # Strict conformance mode
        /Zc:__cplusplus         # Report correct __cplusplus value
        /Zc:preprocessor        # Enable conforming preprocessor
        /utf-8                  # Use UTF-8 encoding
    )
    set_property(TARGET WorkBalance PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
    # Debug-specific options
    target_compile_options(WorkBalance PRIVATE
        $<$<CONFIG:Debug>:/ZI>          # Edit and continue debugging
        $<$<CONFIG:Debug>:/Od>          # Disable optimizations
    )
    # Release-specific options
    target_compile_options(WorkBalance PRIVATE
        $<$<CONFIG:Release>:/O2>        # Maximize speed
        $<$<CONFIG:Release>:/DNDEBUG>   # Define NDEBUG
    )
else()
    target_compile_options(WorkBalance PRIVATE
        -Wall -Wextra -Wpedantic       # Enable warnings
        -Wconversion -Wsign-conversion  # Additional useful warnings
    )
    # Debug-specific options
    target_compile_options(WorkBalance PRIVATE
        $<$<CONFIG:Debug>:-g3>          # Full debug info
        $<$<CONFIG:Debug>:-O0>          # No optimization
    )
    # Release-specific options
    target_compile_options(WorkBalance PRIVATE
        $<$<CONFIG:Release>:-O3>        # Maximum optimization
        $<$<CONFIG:Release>:-DNDEBUG>   # Define NDEBUG
    )
endif()